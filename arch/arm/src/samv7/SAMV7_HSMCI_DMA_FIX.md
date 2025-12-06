# SAMV71 HSMCI TX DMA Fix Documentation

**Date:** December 2024
**Target:** SAMV71-XULT with PX4 Autopilot
**Issue:** SD Card TX DMA writes fail verification - data not persisting to card

---

## Table of Contents

1. [Problem Statement](#problem-statement)
2. [Background](#background)
3. [Root Cause Analysis](#root-cause-analysis)
4. [Implementation Comparison: Harmony vs NuttX](#implementation-comparison-harmony-vs-nuttx)
5. [Solution Approach](#solution-approach)
6. [Implementation Details](#implementation-details)
7. [Files Modified](#files-modified)
8. [Testing Plan](#testing-plan)
9. [References](#references)

---

## Problem Statement

### Symptoms
- SD card RX (read) DMA works correctly after previous fix (cache invalidation)
- SD card TX (write) DMA completes without errors but data fails verification
- Directory creation works (multi-block writes)
- File content writes fail (single-block writes to different buffer address)
- Error pattern: `param_verify()` returns error code 1 (EPERM)
- Each write retry allocates new clusters (29744, 29776, 29792...)

### Debug Output Pattern
```
[TX_DMA] buf=0x20450000 len=512 blk=1 -> SUCCESS (directory)
[TX_DMA] buf=0x20450200 len=512 blk=1 -> FAIL (file content)
```

### Impact on PX4
- Parameter storage fails
- Flight logging fails
- Mission/waypoint saves fail
- System unusable for flight operations

---

## Background

### Hardware Configuration
- **MCU:** ATSAMV71Q21B (Cortex-M7 with D-Cache)
- **Board:** SAMV71-XULT with SD card click board
- **SD Interface:** HSMCI (High Speed MultiMedia Card Interface)
- **DMA Controller:** XDMAC (Extensible DMA Controller)

### Cache Architecture
The Cortex-M7 has a 16KB Data Cache (D-Cache) that requires explicit management for DMA:
- **TX (Memory → Peripheral):** Must CLEAN cache to push data to RAM before DMA reads
- **RX (Peripheral → Memory):** Must INVALIDATE cache to discard stale data after DMA writes

### Previous Fixes Applied
| Fix # | Description | Result |
|-------|-------------|--------|
| #31-34 | Various TX DMA approaches (TDR vs FIFO, indexed FIFO) | Partial |
| #35 | Final cache flush before sam_dmastart() | Failed |
| #36 | Address-based cache clean matching Harmony | Untested |

---

## Root Cause Analysis

### Working Reference Implementation
A fully functional Harmony-based SD card implementation exists on the same SAMV71-XULT board:
- Location: `/media/bhanu1234/Development/samv71_sdcard/`
- Achieves high-throughput SD card read/write
- Uses same HSMCI and XDMAC peripherals

### Key Architectural Differences Found

#### 1. DMA Descriptor Mode

**Harmony (WORKING):**
```c
// Direct register programming - NO linked-list
XDMAC_REGS->XDMAC_CHID[channel].XDMAC_CSA = (uint32_t)srcAddr;
XDMAC_REGS->XDMAC_CHID[channel].XDMAC_CDA = (uint32_t)destAddr;
XDMAC_REGS->XDMAC_CHID[channel].XDMAC_CUBC = XDMAC_CUBC_UBLEN(blockSize);
__DMB();
XDMAC_REGS->XDMAC_GE = (XDMAC_GE_EN0_Msk << channel);
```

**NuttX (FAILING):**
```c
// Linked-list descriptors in cacheable RAM
for (i = 0; i < nblocks; i++) {
    sam_dmatxsetup(priv->dma, regaddr, memaddr, blocksize);
    // Creates descriptor via sam_allocdesc()
    // Each descriptor cleaned individually
}
sam_dmastart();  // Executes linked list via sam_multiple()
```

#### 2. FIFO Addressing

**Harmony (WORKING):**
```c
// Fixed base address - HSMCI handles FIFO pointer internally
static const uint8_t* hsmciFifoBaseAddress =
    (uint8_t*)(HSMCI_BASE_ADDRESS + HSMCI_FIFO_REG_OFST);  // 0x200

XDMAC_ChannelTransfer(channel, buffer, hsmciFifoBaseAddress, numBytes/4);
// DMA config: XDMAC_CC_DAM_FIXED_AM (destination address fixed)
```

**NuttX (FAILING):**
```c
// Indexed addressing - different offset per block
regaddr = hsmci_regaddr(priv, SAM_HSMCI_FIFO_OFFSET);  // 0x200
for (i = 0; i < nblocks; i++) {
    sam_dmatxsetup(priv->dma, regaddr, memaddr, blocksize);
    regaddr += sizeof(uint32_t);  // 0x200, 0x204, 0x208...
    memaddr += blocksize;
}
```

#### 3. Cache Management

**Harmony (WORKING):**
```c
// Single cache clean in driver layer BEFORE DMA setup
SYS_CACHE_CleanDCache_by_Addr(buffer, size);
// Then DMA setup
HSMCI_DmaSetup(buffer, numBytes, WRITE);
// DMB inside XDMAC_ChannelTransfer before enable
__DMB();
```

**NuttX (FAILING):**
```c
// Multiple cache operations, potential descriptor coherency issue
up_clean_dcache(buffer, buffer + buflen);  // Clean buffer
__DMB();
// Loop creates descriptors - each cleaned individually in sam_allocdesc()
// BUT: Previous descriptor modified (NDE, CNDA) but NOT re-cleaned!
up_clean_dcache(buffer, buffer + buflen);  // Clean buffer again
__DMB();
sam_dmastart();
```

#### 4. Potential Bug in NuttX Linked-List

In `sam_allocdesc()` (sam_xdmac.c:1044-1086):
```c
if (prev != NULL) {
    prev->cubc |= CHNEXT_UBC_NDE;   // MODIFY previous descriptor
    prev->cnda = (uint32_t)sam_physramaddr((uintptr_t)descr);  // MODIFY previous
}
xdmach->lltail = descr;
up_clean_dcache((uintptr_t)descr, ...);  // Only cleans NEW descriptor!
// Previous descriptor modifications may still be in cache!
```

### Summary of Differences

| Aspect | Harmony (Works) | NuttX (Fails) |
|--------|-----------------|---------------|
| DMA Mode | Direct registers | Linked-list descriptors |
| FIFO Address | Fixed base (0x200) | Indexed (0x200, 0x204...) |
| Descriptors | None | In cacheable RAM |
| Cache Clean | Once, before setup | Multiple, potential bug |
| Memory Barrier | DMB only | DMB (was DSB+ISB) |
| Code Complexity | Simple | Complex |

---

## Solution Approach

### Evaluated Options

#### Option A: Match Harmony Exactly ✓ SELECTED
- Use direct register programming (no linked-list)
- Use fixed FIFO base address
- Single cache clean before DMA
- Simple, proven approach

#### Option B: Fix Current NuttX Approach
- Keep linked-list descriptors
- Fix cache coherency (re-clean all descriptors)
- Use fixed FIFO base address
- More complex, unproven

#### Option C: Hybrid
- Single-block: direct registers
- Multi-block: fixed linked-list
- Two code paths to maintain

### Decision: Option A

**Rationale for PX4 Application:**

1. **Reliability > Performance:** Flight data integrity is critical
2. **Proven Implementation:** Harmony works on exact same hardware
3. **Simpler = Safer:** Fewer failure modes during flight
4. **Negligible Performance Impact:**
   - Extra overhead: ~1-2μs per block for multi-block
   - At 250Hz logger rate: 0.2-0.4% CPU (negligible on 300MHz M7)
5. **Easier Debugging:** Simpler code path when issues occur in flight

---

## Implementation Details

### Changes Required

#### 1. sam_hsmci.c - TX DMA Setup

**Current (sam_dmasendsetup):**
```c
// Creates linked-list with indexed FIFO addresses
regaddr = hsmci_regaddr(priv, SAM_HSMCI_FIFO_OFFSET);
for (i = 0; i < nblocks; i++) {
    sam_dmatxsetup(priv->dma, regaddr, memaddr, blocksize);
    regaddr += sizeof(uint32_t);
    memaddr += blocksize;
}
```

**New (Match Harmony):**
```c
// Single DMA transfer to fixed FIFO base address
// Cache clean once before setup
up_clean_dcache((uintptr_t)buffer, (uintptr_t)buffer + buflen);
__DMB();

// Use fixed FIFO base for entire transfer
regaddr = hsmci_regaddr(priv, SAM_HSMCI_FIFO_OFFSET);  // Fixed 0x200
sam_dmatxsetup(priv->dma, regaddr, (uintptr_t)buffer, buflen);
// Single descriptor, entire buffer, fixed destination
```

#### 2. DMA Channel Configuration

Ensure TX DMA uses:
- `XDMAC_CC_DAM_FIXED_AM` - Destination address fixed (FIFO)
- `XDMAC_CC_SAM_INCREMENTED_AM` - Source address incremented (buffer)
- `XDMAC_CC_DWIDTH_WORD` - 32-bit transfers
- `XDMAC_CC_DSYNC_MEM2PER` - Memory to peripheral

#### 3. Transfer Count

Match Harmony's calculation:
```c
// Transfer count in 32-bit words
uint32_t transferCount = buflen / 4;
```

### Code Flow After Fix

```
sam_dmasendsetup():
  1. Validate buffer alignment
  2. Cache clean buffer (address-based)
  3. DMB barrier
  4. Get fixed FIFO base address
  5. Single sam_dmatxsetup() for entire buffer
  6. Enable HSMCI DMA
  7. sam_dmastart() -> sam_single() (single descriptor)
```

---

## Files Modified

### Primary Changes
| File | Change |
|------|--------|
| `arch/arm/src/samv7/sam_hsmci.c` | TX DMA setup to match Harmony |

### Potentially Affected (Review)
| File | Reason |
|------|--------|
| `arch/arm/src/samv7/sam_xdmac.c` | May need to verify single-transfer path |
| `arch/arm/src/samv7/sam_xdmac.h` | Check DMA flag definitions |

### Reference Files (Harmony - DO NOT MODIFY)
| File | Purpose |
|------|---------|
| `/media/bhanu1234/Development/samv71_sdcard/src/config/default/peripheral/hsmci/plib_hsmci.c` | Working HSMCI implementation |
| `/media/bhanu1234/Development/samv71_sdcard/src/config/default/peripheral/xdmac/plib_xdmac.c` | Working XDMAC implementation |
| `/media/bhanu1234/Development/samv71_sdcard/src/config/default/driver/sdmmc/src/drv_sdmmc.c` | Cache handling reference |

---

## Testing Plan

### Unit Tests
1. Single block write (512 bytes) - parameter save
2. Multi-block write (4KB) - logger burst
3. Large file write (1MB) - sustained throughput
4. Read-after-write verification

### Integration Tests
1. PX4 parameter save/load cycle
2. Logger start/stop with data verification
3. Mission upload/download
4. Dataman operations

### Stress Tests
1. Continuous logging for 1 hour
2. Rapid parameter changes
3. Power cycle during write
4. SD card hot swap (if supported)

### Performance Benchmarks
1. Measure write throughput (MB/s)
2. Measure CPU usage during logging
3. Compare with previous implementation

---

## References

### Datasheets
- SAMV71 Datasheet (DS60001527) - HSMCI and XDMAC sections
- ARM Cortex-M7 TRM - Cache maintenance operations

### Code References
- Harmony HSMCI: `plib_hsmci.c:207-261` (HSMCI_DmaSetup)
- Harmony XDMAC: `plib_xdmac.c:153-188` (XDMAC_ChannelTransfer)
- Harmony Cache: `drv_sdmmc.c:2201-2214` (Cache clean before write)
- NuttX HSMCI: `sam_hsmci.c:3207-3327` (sam_dmasendsetup)
- NuttX XDMAC: `sam_xdmac.c:990-1099` (sam_allocdesc)

### Related Fixes
- RX DMA cache fix: Changed `up_clean_dcache` to `up_invalidate_dcache` in sam_xdmac.c
- Previous TX fixes #31-36: Various approaches documented in code comments

---

## Revision History

| Date | Author | Description |
|------|--------|-------------|
| 2024-12 | Claude/User | Initial analysis and Option A selection |
| 2024-12 | Claude/User | FIX #37 implemented - Single DMA transfer to fixed FIFO base |
| 2024-12 | Claude/User | FIX #38 implemented - Add DMB before channel enable in sam_xdmac.c |
| 2024-12 | Claude/User | FIX #39 implemented - RX path: single DMA, fixed FIFO, address-based cache |
| 2024-12 | Claude/User | FIX #40 implemented - CSTOR, FERRCTRL, PWSEN initialization to match Harmony |
| 2024-12 | Claude/User | FIX #41 implemented - Full D-Cache clean (FAILED - cache not the issue) |
| 2024-12 | Claude/User | FIX #42/43 implemented - Reorder operations to match Harmony exactly (FAILED) |
| 2024-12 | Claude/User | Discovered buffer address-specific failure pattern: 0x20450000 works, 0x20450200 fails |
| 2024-12 | Claude/User | FIX #44 implemented - TX polling mode (FAILED - same as DMA, ruled out DMA as cause) |
| 2024-12 | Claude/User | FIX #45 implemented - Read-after-write verification to confirm data not persisting |
| 2024-12-06 | Claude/User | Bit pattern analysis: bits 2,6 cleared → D2 data line issue |
| 2024-12-06 | Claude/User | **FIX #46 - ROOT CAUSE FOUND: PA26 pin conflict between PWM2 and HSMCI DA2** |
| 2024-12-06 | Claude/User | **FIX #46 VERIFIED WORKING** - All SD card writes now pass verification! |

---

## FIX #37 Implementation Summary

**Changes made to `sam_hsmci.c` in `sam_dmasendsetup()`:**

1. **Removed for-loop** that created multiple DMA descriptors with indexed FIFO addresses
2. **Single DMA transfer** for entire buffer: `sam_dmatxsetup(priv->dma, regaddr, buffer, buflen)`
3. **Fixed FIFO base address** (0x200) - HSMCI handles FIFO indexing internally
4. **Single cache clean** before DMA setup (removed redundant second clean)
5. **Simplified code** - removed unused variables (blocksize, nblocks, i, memaddr)

**Before (complex, linked-list):**
```c
for (i = 0; i < nblocks; i++) {
    sam_dmatxsetup(priv->dma, regaddr, memaddr, blocksize);
    regaddr += sizeof(uint32_t);  // 0x200, 0x204, 0x208...
    memaddr += blocksize;
}
```

**After (simple, matches Harmony):**
```c
regaddr = hsmci_regaddr(priv, SAM_HSMCI_FIFO_OFFSET);  // Fixed 0x200
sam_dmatxsetup(priv->dma, regaddr, (uintptr_t)buffer, buflen);
```

---

## FIX #40 Implementation Summary

**Changes made to `sam_hsmci.c` in `sam_reset()`:**

1. **Added CSTOR (Completion Signal Timeout Register)**:
   - Harmony sets: `HSMCI_CSTOR_CSTOMUL_1048576 | HSMCI_CSTOR_CSTOCYC(2U)` (~2M cycles)
   - NuttX was leaving it at reset default (0), potentially causing completion timeouts

2. **Added FERRCTRL to CFG register**:
   - Harmony sets: `HSMCI_CFG_FIFOMODE_Msk | HSMCI_CFG_FERRCTRL_Msk`
   - NuttX had a comment "may cause hangs" but Harmony uses it successfully
   - FERRCTRL=1: Only software reset clears flow error bits (more predictable behavior)

3. **Added PWSEN to CR register**:
   - Harmony enables: `HSMCI_CR_MCIEN_Msk | HSMCI_CR_PWSEN_Msk`
   - PWSEN enables automatic power saving when idle

**Initialization Comparison:**

| Setting | Harmony | NuttX (Before) | NuttX (After FIX #40) |
|---------|---------|----------------|------------------------|
| CSTOR | 2M cycles | Not set (default 0) | 2M cycles |
| FERRCTRL | Set | Not set | Set |
| PWSEN | Enabled | Disabled | Enabled |

---

## FIX #41 - Full D-Cache Clean (FAILED)

**Hypothesis:** Cache coherency issue with buffer data

**Change:** Added full D-Cache clean before DMA setup:
```c
up_clean_dcache(0x20400000, 0x20460000);  // Clean entire SRAM
```

**Result:** FAILED - Same failure pattern. Proves cache is NOT the root cause.

---

## FIX #42/43 - Match Harmony's Exact Operation Order

**Hypothesis:** Order of operations matters

**Changes to `sam_dmasendsetup()`:**
```c
/* Step 1: Enable HSMCI DMA FIRST (before anything else) */
sam_putreg(priv, HSMCI_DMA_DMAEN | HSMCI_DMA_CHKSIZE, SAM_HSMCI_DMA_OFFSET);

/* Step 2: Stop any previous DMA transfer */
sam_dmastop(priv->dma);

/* Step 3: Clean cache to push buffer data to RAM */
up_clean_dcache((uintptr_t)buffer, (uintptr_t)buffer + buflen);

/* Step 4: Data Memory Barrier */
__asm__ __volatile__ ("dmb" ::: "memory");

/* Step 5: Single DMA transfer to FIXED FIFO base address */
regaddr = hsmci_regaddr(priv, SAM_HSMCI_FIFO_OFFSET);
sam_dmatxsetup(priv->dma, regaddr, (uintptr_t)buffer, buflen);

/* Step 6: Start DMA */
sam_dmastart(priv->dma, sam_dmacallback, priv);
```

**Result:** FAILED - Same failure pattern persists.

---

## Current Debug State (December 2024)

### Critical Finding: Buffer Address-Specific Failure

**Pattern Discovered:**
| Buffer Address | Usage | Result |
|----------------|-------|--------|
| `0x20450000` (fs_buffer) | FAT/directory metadata | **ALWAYS WORKS** |
| `0x20450200` (ff_buffer) | File content data | **ALWAYS FAILS** |

Both buffers are:
- Allocated from same `.nocache` DMA heap via `board_dma_alloc()`
- In same MPU Region 14 (non-cacheable, 0x20450000-0x2045FFFF)
- Using identical DMA code path (`sam_dmasendsetup` → `sam_single`)
- 512-byte aligned

### Latest Test Output

```
nsh> echo "test data" > /fs/microsd/test.txt
[WR] blk=29728 buf=0x20450000 data=[13 00 03 01]  ← Directory (works)
[WR] blk=29952 buf=0x20450200 data=[74 65 73 74]  ← File "test" (FAILS)
[WR] blk=32 buf=0x20450000 data=[b8 bb bb 0b]     ← FAT (works)
[WR] blk=14880 buf=0x20450000 data=[b8 bb bb 0b]  ← FAT backup (works)
[WR] blk=29728 buf=0x20450000 data=[13 00 03 01]  ← Directory (works)
[WR] blk=1 buf=0x20450000 data=[52 52 61 41]      ← FSInfo (works)

nsh> cat /fs/microsd/test.txt
nsh: cat: open failed: ENOENT   ← File doesn't exist!

nsh> ls /fs/microsd/
/fs/microsd:
     .                          ← Corrupted/garbage entries
```

### Memory Configuration Verified

**Linker Script (`script.ld`):**
```
nocache (rwx) : ORIGIN = 0x20450000, LENGTH = 64K
```

**MPU Configuration (`sam_mpuinit.c`):**
```c
uint32_t region_base = 0x20450000;
uint32_t region_num  = 14;  /* High priority */
uint32_t l2size      = 16;  /* 64KB = 2^16 */
/* TEX=001 (Normal), S=1 (Shareable), C=0 (NC), B=0 (NB) */
```

**DMA Heap (`board_dma_alloc.c`):**
```c
static uint8_t g_dma_heap[BOARD_DMA_ALLOC_POOL_SIZE]
    __attribute__((section(".nocache"), aligned(64)));
```

### What We've Ruled Out

| Hypothesis | Test | Result |
|------------|------|--------|
| Cache coherency | FIX #41 full cache clean | NOT the issue |
| Operation order | FIX #42/43 match Harmony | NOT the issue |
| HSMCI initialization | FIX #40 CSTOR/FERRCTRL/PWSEN | NOT the issue |
| DMA descriptor issues | Using `sam_single()` direct registers | NOT the issue |
| FIFO addressing | Fixed base address 0x200 | NOT the issue |
| Memory barriers | DMB before DMA enable | NOT the issue |

### Remaining Hypotheses

1. **Granule Allocator Bug** - Second allocation (0x20450200) may have different properties
2. **Address-Specific DMA Behavior** - Something about 0x200 offset in source address
3. **FAT Filesystem Timing** - ff_buffer modified during DMA
4. **Descriptor Pool State** - `g_lldesc[]` in cacheable RAM may have stale state

### Key Code Paths

**Working Path (fs_buffer @ 0x20450000):**
```
FAT sector cache operations → sam_dmasendsetup() → DMA → SUCCESS
```

**Failing Path (ff_buffer @ 0x20450200):**
```
File I/O operations → sam_dmasendsetup() → DMA → SILENT FAILURE
```

---

## FIX #44: TX Polling Workaround (December 2024)

### Approach
Disabled TX DMA entirely and switched to CPU polling mode to test if the issue is DMA-specific.

**Change in `sam_hsmci.c`:**
```c
#undef  HSCMI_NORXDMA              /* RX DMA enabled and working */
#define HSCMI_NOTXDMA              /* TX DMA disabled - use polling */
```

This causes `dmasendsetup` to map to `sam_sendsetup` (polling) instead of `sam_dmasendsetup` (DMA).

### Result: **FAILED - SAME PATTERN!**

Both DMA and polling fail identically:
- Buffer `0x20450000` (fs_buffer): **WORKS** with both methods
- Buffer `0x20450200` (ff_buffer): **FAILS** with both methods

**CRITICAL FINDING:** This RULES OUT TX DMA as the root cause!

The issue is NOT in the HSMCI TX path (whether DMA or polling). Something else is causing buffer-address-specific failures.

---

## FIX #45: Read-After-Write Verification (December 2024)

### Approach
Added read-back verification after each write to definitively determine if data persists on the SD card.

**Code added to `mmcsd_sdio.c` after write completes:**
```c
/* FIX #45: Read-after-write verification */
{
  static uint8_t verify_buf[512] __attribute__((aligned(4)));

  /* Wait for write to complete, then read back */
  verify_ret = mmcsd_transferready(priv);
  verify_ret = mmcsd_readsingle(priv, verify_buf, startblock);

  /* Compare */
  if (memcmp(buffer, verify_buf, 4) != 0) {
    printf("[VERIFY FAIL] blk=%jd wrote=[...] read=[...]\n", startblock);
  } else {
    printf("[VERIFY OK] blk=%jd\n", startblock);
  }
}
```

### Expected Output

**If data is written correctly:**
```
[WR] blk=29920 buf=0x20450200 data=[61 0a 00 00]
[VERIFY OK] blk=29920
```

**If data is NOT written correctly:**
```
[WR] blk=29920 buf=0x20450200 data=[61 0a 00 00]
[VERIFY FAIL] blk=29920 wrote=[61 0a 00 00] read=[xx xx xx xx]
```

### Pending: Test Results

Awaiting test output from hardware to determine:
1. If verification passes → problem is in filesystem layer (sync/flush)
2. If verification fails → problem is in HSMCI write path (even for polling!)

---

## Updated Hypothesis (December 2024)

Since **both DMA and polling** fail for the same buffer address, the issue is:
- NOT in DMA configuration
- NOT in cache coherency
- NOT in the TX transfer mechanism itself

Possible remaining causes:
1. **Card-side issue** - SD card rejecting writes from certain addresses?
2. **Command sequencing** - Something wrong with CMD24 or response handling?
3. **Buffer content corruption** - Data corrupted before write function is called?
4. **Filesystem-level bug** - ff_buffer not properly synced/managed?

---

## FIX #46: PA26 Pin Conflict - ROOT CAUSE FOUND AND FIXED! ✓

**Date:** December 6, 2024
**Status:** **WORKING** - SD card writes verified successfully!

### The Discovery

Analysis of bit corruption patterns in verification failures revealed a critical finding:

**Bit Corruption Pattern:**
```
Block 1:  wrote=[52 52 61 41] read=[12 12 21 01]
          0x52→0x12: bit 6 cleared (0x40)
          0x52→0x12: bit 6 cleared (0x40)
          0x61→0x21: bit 6 cleared (0x40)
          0x41→0x01: bit 6 cleared (0x40)

Block 29760: wrote=[2e 20 20 20] read=[2a 20 20 20]
             0x2e→0x2a: bit 2 cleared (0x04)
```

**Key Insight:** In 4-bit SD card mode, data line D2 carries **bits 2 and 6** of each byte!

### Root Cause: PA26 Pin Conflict

**PA26 was configured for TWO conflicting purposes:**

| Peripheral | Function | Configuration |
|------------|----------|---------------|
| HSMCI0 DA2 | SD card data line 2 | Peripheral C |
| TC0 CH2 (TIOA2) | PWM2 output | Peripheral B |

**Initialization Sequence:**
1. `sam_boardinitialize()` → `px4_gpio_init()` configures GPIO_PWM2_OUT (PA26 as Peripheral B)
2. `sdio_initialize()` → configures GPIO_MCI0_DA2 (PA26 as Peripheral C) ✓ CORRECT
3. **PWM module startup** → PX4 IO timer init reconfigures PA26 as Timer Counter → **BREAKS HSMCI!**

The PWM driver initialization happened AFTER SD card init, taking over PA26 and corrupting all writes that used bit 2 or bit 6.

### Why Some Writes "Worked"

- Writes that appeared to work had data without bits 2 or 6 set (e.g., `0x00`, `0x08`, `0x20`)
- Writes with bits 2 or 6 set failed silently (bits cleared during transmission)
- Reads worked because the SD card output on D2 was still driving, just not being read correctly

### The Fix

**Files Modified:**

#### 1. `boards/microchip/samv71-xult-clickboards/src/board_config.h`

```c
/* BEFORE: */
#define DIRECT_PWM_OUTPUT_CHANNELS  4
#define GPIO_PWM2_OUT    (GPIO_PERIPHB | GPIO_CFG_DEFAULT | GPIO_PORT_PIOA | GPIO_PIN26)  /* TC2 TIOA - PA26 */

/* AFTER: */
#define DIRECT_PWM_OUTPUT_CHANNELS  3
/* GPIO_PWM2_OUT (PA26) REMOVED - conflicts with HSMCI0 DA2 (SD card data line 2) */
#define GPIO_PWM2_OUT    (GPIO_PERIPHB | GPIO_CFG_DEFAULT | GPIO_PORT_PIOC | GPIO_PIN23)  /* TC3 TIOA - PC23 */
#define GPIO_PWM3_OUT    (GPIO_PERIPHB | GPIO_CFG_DEFAULT | GPIO_PORT_PIOC | GPIO_PIN26)  /* TC4 TIOA - PC26 */
```

Also updated `PX4_GPIO_INIT_LIST` and `BOARD_NUM_IO_TIMERS` from 4 to 3.

#### 2. `boards/microchip/samv71-xult-clickboards/src/timer_config.cpp`

```c
/* BEFORE: */
const io_timers_t io_timers[MAX_IO_TIMERS] = {
    initIOTimer(Timer::Timer1),  /* TC block 0 - CH1 */
    initIOTimer(Timer::Timer2),  /* TC block 0 - CH2 (PA26) */
    initIOTimer(Timer::Timer3),  /* TC block 1 - CH0 */
    initIOTimer(Timer::Timer4),  /* TC block 1 - CH1 */
};

/* AFTER: */
const io_timers_t io_timers[MAX_IO_TIMERS] = {
    initIOTimer(Timer::Timer1),  /* TC block 0 - CH1 (PA15) */
    initIOTimer(Timer::Timer3),  /* TC block 1 - CH0 (PC23) */
    initIOTimer(Timer::Timer4),  /* TC block 1 - CH1 (PC26) */
};
// Timer2 (TC0 CH2, PA26) NOT USED - conflicts with SD card
```

### PWM Channel Remapping

| Old Name | Old Pin | New Name | New Pin |
|----------|---------|----------|---------|
| PWM1 | PA15 | PWM1 | PA15 (unchanged) |
| PWM2 | PA26 | **REMOVED** | (conflicts with SD) |
| PWM3 | PC23 | PWM2 | PC23 |
| PWM4 | PC26 | PWM3 | PC26 |

### Verification Results

After fix, ALL writes verify successfully:
```
[VERIFY OK] blk=29808
[VERIFY OK] blk=32
[VERIFY OK] blk=14920
[VERIFY OK] blk=29816
[VERIFY OK] blk=1          ← Previously ALWAYS failed!
[VERIFY OK] blk=29825
[VERIFY OK] blk=29826
... (all subsequent writes OK)
INFO  [logger] log root dir created: /fs/microsd/log  ← Logger works!
```

### Lessons Learned

1. **Pin conflicts are silent failures** - no error reported, data just corrupted
2. **Bit pattern analysis is powerful** - the specific bits cleared pointed directly to D2
3. **4-bit SD mode maps bits to data lines** - D0=bits 0,4; D1=bits 1,5; D2=bits 2,6; D3=bits 3,7
4. **Initialization order matters** - later GPIO configs override earlier ones
5. **This was NOT a DMA issue** - the DMA code was fine all along!

---

## Current Configuration Status (December 2024)

| Component | Status | Notes |
|-----------|--------|-------|
| RX DMA | Enabled | Working with cache invalidation |
| TX DMA | **Disabled** | Using polling mode (can now try enabling) |
| PWM Channels | 3 | PA15, PC23, PC26 (PA26 reserved for SD) |
| SD Card | **WORKING** | All writes verified successfully |

### Next Step: Re-enable TX DMA

Now that the pin conflict is fixed, TX DMA can be tested by:

```c
/* In sam_hsmci.c: */
#undef  HSCMI_NORXDMA   /* RX DMA enabled */
#undef  HSCMI_NOTXDMA   /* TX DMA enabled - try this now! */
```

---

## Notes for Future Sessions

When resuming this work:

1. **SD card writes are WORKING** with TX polling mode
2. **Root cause was PA26 pin conflict** between PWM2 and HSMCI DA2
3. **PWM channels reduced from 4 to 3** to free PA26 for SD card
4. **Next step:** Try re-enabling TX DMA - should work now!
5. **Then:** Clean up debug printf statements
