/*
 * Copyright (c) 2017 comsuisse AG
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * @addtogroup t_i2s_api
 * @{
 * @defgroup t_i2s_loopback test_i2s_loopback
 * @brief TestPurpose: verify I2S master can read and write in loopback mode
 * @}
 */

#include <ugelis.h>
#include <ztest.h>
#include <i2s.h>

#define I2S_DEV_NAME "I2S_0"
#define NUM_RX_BLOCKS 2
#define NUM_TX_BLOCKS 2
#define SAMPLE_NO 32

/* The data_l represent a sine wave */
static s16_t data_l[SAMPLE_NO] = {
	  6392,  12539,  18204,  23169,  27244,  30272,  32137,  32767,  32137,
	 30272,  27244,  23169,  18204,  12539,   6392,      0,  -6393, -12540,
	-18205, -23170, -27245, -30273, -32138, -32767, -32138, -30273, -27245,
	-23170, -18205, -12540,  -6393,     -1,
};

/* The data_r represent a sine wave with double the frequency of data_l */
static s16_t data_r[SAMPLE_NO] = {
	 12539,  23169,  30272,  32767,  30272,  23169,  12539,      0, -12540,
	-23170, -30273, -32767, -30273, -23170, -12540,     -1,  12539,  23169,
	 30272,  32767,  30272,  23169,  12539,      0, -12540, -23170, -30273,
	-32767, -30273, -23170, -12540,     -1,
};

#define BLOCK_SIZE (2 * sizeof(data_l))

K_MEM_SLAB_DEFINE(rx_0_mem_slab, BLOCK_SIZE, NUM_RX_BLOCKS, 1);
K_MEM_SLAB_DEFINE(tx_0_mem_slab, BLOCK_SIZE, NUM_TX_BLOCKS, 1);

static void fill_buf(s16_t *tx_block, int att)
{
	for (int i = 0; i < SAMPLE_NO; i++) {
		tx_block[2 * i] = data_l[i] >> att;
		tx_block[2 * i + 1] = data_r[i] >> att;
	}
}

static int verify_buf(s16_t *rx_block, int att)
{
	for (int i = 0; i < SAMPLE_NO; i++) {
		if (rx_block[2 * i] != data_l[i] >> att) {
			TC_PRINT("Error: att %d: data_l mismatch at position "
				 "%d, expected %d, actual %d\n",
				 att, i, data_l[i] >> att, rx_block[2 * i]);
			return -TC_FAIL;
		}
		if (rx_block[2 * i + 1] != data_r[i] >> att) {
			TC_PRINT("Error: att %d: data_r mismatch at position "
				 "%d, expected %d, actual %d\n",
				 att, i, data_r[i] >> att, rx_block[2 * i + 1]);
			return -TC_FAIL;
		}
	}

	return TC_PASS;
}

static void fill_buf_const(s16_t *tx_block, s16_t val_l, s16_t val_r)
{
	for (int i = 0; i < SAMPLE_NO; i++) {
		tx_block[2 * i] = val_l;
		tx_block[2 * i + 1] = val_r;
	}
}

static int verify_buf_const(s16_t *rx_block, s16_t val_l, s16_t val_r)
{
	for (int i = 0; i < SAMPLE_NO; i++) {
		if (rx_block[2 * i] != val_l) {
			TC_PRINT("Error: data_l mismatch at position "
				 "%d, expected %d, actual %d\n",
				 i, val_l, rx_block[2 * i]);
			return -TC_FAIL;
		}
		if (rx_block[2 * i + 1] != val_r) {
			TC_PRINT("Error: data_r mismatch at position "
				 "%d, expected %d, actual %d\n",
				 i, val_r, rx_block[2 * i + 1]);
			return -TC_FAIL;
		}
	}

	return TC_PASS;
}

static int tx_block_write(struct device *dev_i2s, int att, int err)
{
	void *tx_block;
	int ret;

	ret = k_mem_slab_alloc(&tx_0_mem_slab, &tx_block, K_FOREVER);
	if (ret < 0) {
		TC_PRINT("Error: Failed to allocate tx_block\n");
		return -TC_FAIL;
	}
	fill_buf((u16_t *)tx_block, att);
	ret = i2s_write(dev_i2s, tx_block, BLOCK_SIZE);
	if (ret < 0) {
		k_mem_slab_free(&tx_0_mem_slab, &tx_block);
	}
	if (ret != err) {
		TC_PRINT("Error: i2s_write failed expected %d, actual %d\n",
			 err, ret);
		return -TC_FAIL;
	}

	return TC_PASS;
}

static int rx_block_read(struct device *dev_i2s, int att)
{
	void *rx_block;
	size_t rx_size;
	int ret;

	ret = i2s_read(dev_i2s, &rx_block, &rx_size);
	if (ret < 0 || rx_size != BLOCK_SIZE) {
		TC_PRINT("Error: Read failed\n");
		return -TC_FAIL;
	}
	ret = verify_buf((u16_t *)rx_block, att);
	if (ret < 0) {
		TC_PRINT("Error: Verify failed\n");
		return -TC_FAIL;
	}
	k_mem_slab_free(&rx_0_mem_slab, &rx_block);

	return TC_PASS;
}

#define TIMEOUT          2000
#define FRAME_CLK_FREQ   8000

/** Configure I2S TX transfer. */
void test_i2s_tx_transfer_configure_0(void)
{
	struct device *dev_i2s;
	struct i2s_config i2s_cfg;
	int ret;

	dev_i2s = device_get_binding(I2S_DEV_NAME);
	zassert_not_null(dev_i2s, "device " I2S_DEV_NAME " not found");

	/* Configure */

	i2s_cfg.word_size = 16;
	i2s_cfg.channels = 2;
	i2s_cfg.format = I2S_FMT_DATA_FORMAT_I2S;
	i2s_cfg.options = I2S_OPT_FRAME_CLK_SLAVE | I2S_OPT_BIT_CLK_SLAVE;
	i2s_cfg.frame_clk_freq = FRAME_CLK_FREQ;
	i2s_cfg.block_size = BLOCK_SIZE;
	i2s_cfg.mem_slab = &tx_0_mem_slab;
	i2s_cfg.timeout = TIMEOUT;
	i2s_cfg.options = I2S_OPT_LOOPBACK;

	ret = i2s_configure(dev_i2s, I2S_DIR_TX, &i2s_cfg);
	zassert_equal(ret, 0, "Failed to configure I2S TX stream");
}

/** Configure I2S RX transfer. */
void test_i2s_rx_transfer_configure_0(void)
{
	struct device *dev_i2s;
	struct i2s_config i2s_cfg;
	int ret;

	dev_i2s = device_get_binding(I2S_DEV_NAME);
	zassert_not_null(dev_i2s, "device " I2S_DEV_NAME " not found");

	/* Configure */

	i2s_cfg.word_size = 16;
	i2s_cfg.channels = 2;
	i2s_cfg.format = I2S_FMT_DATA_FORMAT_I2S;
	i2s_cfg.options = I2S_OPT_FRAME_CLK_SLAVE | I2S_OPT_BIT_CLK_SLAVE;
	i2s_cfg.frame_clk_freq = FRAME_CLK_FREQ;
	i2s_cfg.block_size = BLOCK_SIZE;
	i2s_cfg.mem_slab = &rx_0_mem_slab;
	i2s_cfg.timeout = TIMEOUT;
	i2s_cfg.options = I2S_OPT_LOOPBACK;

	ret = i2s_configure(dev_i2s, I2S_DIR_RX, &i2s_cfg);
	zassert_equal(ret, 0, "Failed to configure I2S RX stream");
}

/** @brief Short I2S transfer.
 *
 * - TX stream START trigger starts transmission.
 * - RX stream START trigger starts reception.
 * - sending / receiving a short sequence of data returns success.
 * - TX stream DRAIN trigger empties the transmit queue.
 * - RX stream STOP trigger stops reception.
 */
void test_i2s_transfer_short(void)
{
	struct device *dev_i2s;
	int ret;

	dev_i2s = device_get_binding(I2S_DEV_NAME);
	zassert_not_null(dev_i2s, "device " I2S_DEV_NAME " not found");

	/* Prefill TX queue */
	ret = tx_block_write(dev_i2s, 0, 0);
	zassert_equal(ret, TC_PASS, NULL);
	TC_PRINT("%d->OK\n", 1);

	ret = tx_block_write(dev_i2s, 1, 0);
	zassert_equal(ret, TC_PASS, NULL);
	TC_PRINT("%d->OK\n", 2);

	/* Start reception */
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "RX START trigger failed");

	/* Start transmission */
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "TX START trigger failed");

	ret = rx_block_read(dev_i2s, 0);
	zassert_equal(ret, TC_PASS, NULL);
	TC_PRINT("%d<-OK\n", 1);

	ret = tx_block_write(dev_i2s, 2, 0);
	zassert_equal(ret, TC_PASS, NULL);
	TC_PRINT("%d->OK\n", 3);

	/* All data written, drain TX queue and stop the transmission */
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_DRAIN);
	zassert_equal(ret, 0, "TX DRAIN trigger failed");

	ret = rx_block_read(dev_i2s, 1);
	zassert_equal(ret, TC_PASS, NULL);
	TC_PRINT("%d<-OK\n", 2);

	/* All but one data block read, stop reception */
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_STOP);
	zassert_equal(ret, 0, "RX STOP trigger failed");

	ret = rx_block_read(dev_i2s, 2);
	zassert_equal(ret, TC_PASS, NULL);
	TC_PRINT("%d<-OK\n", 3);

	/* TODO: Verify the interface is in READY state when i2s_state_get
	 * function is available.
	 */
}

#define TEST_I2S_TRANSFER_LONG_REPEAT_COUNT  100

/** @brief Long I2S transfer.
 *
 * - TX stream START trigger starts transmission.
 * - RX stream START trigger starts reception.
 * - sending / receiving a long sequence of data returns success.
 * - TX stream DRAIN trigger empties the transmit queue.
 * - RX stream STOP trigger stops reception.
 */
void test_i2s_transfer_long(void)
{
	struct device *dev_i2s;
	int ret;

	dev_i2s = device_get_binding(I2S_DEV_NAME);
	zassert_not_null(dev_i2s, "device " I2S_DEV_NAME " not found");

	/* Prefill TX queue */
	ret = tx_block_write(dev_i2s, 0, 0);
	zassert_equal(ret, TC_PASS, NULL);

	/* Start reception */
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "RX START trigger failed");

	/* Start transmission */
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "TX START trigger failed");

	for (int i = 0; i < TEST_I2S_TRANSFER_LONG_REPEAT_COUNT; i++) {
		ret = tx_block_write(dev_i2s, 0, 0);
		zassert_equal(ret, TC_PASS, NULL);

		ret = rx_block_read(dev_i2s, 0);
		zassert_equal(ret, TC_PASS, NULL);
	}

	/* All data written, flush TX queue and stop the transmission */
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_DRAIN);
	zassert_equal(ret, 0, "TX DRAIN trigger failed");

	/* All but one data block read, stop reception */
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_STOP);
	zassert_equal(ret, 0, "RX STOP trigger failed");

	ret = rx_block_read(dev_i2s, 0);
	zassert_equal(ret, TC_PASS, NULL);

	/* TODO: Verify the interface is in READY state when i2s_state_get
	 * function is available.
	 */
}

/** @brief RX sync start.
 *
 * - TX stream START trigger starts transmission.
 * - sending RX stream START trigger after a delay starts reception on the next
 *   word select sync event at the start of the frame.
 * - TX stream DROP trigger stops transmission and clears the transmit queue.
 * - RX stream DROP trigger stops reception and clears the receive queue.
 */
void test_i2s_rx_sync_start(void)
{
	struct device *dev_i2s;
	void *tx_block;
	void *rx_block;
	size_t rx_size;
	int ret;

	dev_i2s = device_get_binding(I2S_DEV_NAME);
	zassert_not_null(dev_i2s, "device " I2S_DEV_NAME " not found");

	/* Prefill TX queue */
	for (int n = 0; n < NUM_TX_BLOCKS; n++) {
		ret = k_mem_slab_alloc(&tx_0_mem_slab, &tx_block, K_FOREVER);
		zassert_equal(ret, TC_PASS, NULL);
		fill_buf_const((u16_t *)tx_block, 1, 2);
		ret = i2s_write(dev_i2s, tx_block, BLOCK_SIZE);
		zassert_equal(ret, TC_PASS, NULL);
		TC_PRINT("%d->OK\n", n);
	}

	/* Start transmission */
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "TX START trigger failed");

	k_busy_wait(75);

	/* Start reception */
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "RX START trigger failed");

	ret = i2s_read(dev_i2s, &rx_block, &rx_size);
	zassert_equal(ret, TC_PASS, NULL);
	ret = verify_buf_const((u16_t *)rx_block, 1, 2);
	k_mem_slab_free(&rx_0_mem_slab, &rx_block);
	zassert_equal(ret, TC_PASS, NULL);
	TC_PRINT("%d<-OK\n", 1);

	/* All data written, drop TX, RX queue and stop the transmission */
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_DROP);
	zassert_equal(ret, 0, "TX DROP trigger failed");

	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_DROP);
	zassert_equal(ret, 0, "RX DROP trigger failed");

	/* TODO: Verify the interface is in READY state when i2s_state_get
	 * function is available.
	 */
}

/** @brief Timeout on RX queue empty.
 *
 * - Reading empty RX queue in READY state returns time out error.
 */
void test_i2s_rx_empty_timeout(void)
{
	struct device *dev_i2s;
	void *rx_block;
	size_t rx_size;
	int ret;

	dev_i2s = device_get_binding(I2S_DEV_NAME);
	zassert_not_null(dev_i2s, "device " I2S_DEV_NAME " not found");

	ret = i2s_read(dev_i2s, &rx_block, &rx_size);
	zassert_equal(ret, -EAGAIN, "i2s_read did not timed out");
}

#define TEST_I2S_TRANSFER_RESTART_PAUSE_LENGTH_US  1000

/** @brief Re-start I2S transfer.
 *
 * - STOP trigger stops transfer / reception at the end of the current block,
 *   consecutive START trigger restarts transfer / reception with the next data
 *   block.
 */
void test_i2s_transfer_restart(void)
{
	struct device *dev_i2s;
	int ret;

	dev_i2s = device_get_binding(I2S_DEV_NAME);
	zassert_not_null(dev_i2s, "device " I2S_DEV_NAME " not found");

	/* Prefill TX queue */
	ret = tx_block_write(dev_i2s, 0, 0);
	zassert_equal(ret, TC_PASS, NULL);
	TC_PRINT("%d->OK\n", 1);

	ret = tx_block_write(dev_i2s, 1, 0);
	zassert_equal(ret, TC_PASS, NULL);
	TC_PRINT("%d->OK\n", 2);

	/* Start reception */
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "RX START trigger failed");

	/* Start transmission */
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "TX START trigger failed");

	/* Stop transmission */
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_STOP);
	zassert_equal(ret, 0, "TX STOP trigger failed");

	/* Stop reception */
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_STOP);
	zassert_equal(ret, 0, "RX STOP trigger failed");

	ret = rx_block_read(dev_i2s, 0);
	zassert_equal(ret, TC_PASS, NULL);
	TC_PRINT("%d<-OK\n", 1);

	TC_PRINT("Stop transmission\n");

	/* Keep interface inactive */
	k_sleep(TEST_I2S_TRANSFER_RESTART_PAUSE_LENGTH_US);

	TC_PRINT("Start transmission\n");

	/* Prefill TX queue */
	ret = tx_block_write(dev_i2s, 2, 0);
	zassert_equal(ret, TC_PASS, NULL);
	TC_PRINT("%d->OK\n", 3);

	/* Start reception */
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "RX START trigger failed");

	/* Start transmission */
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "TX START trigger failed");

	/* All data written, drain TX queue and stop the transmission */
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_DRAIN);
	zassert_equal(ret, 0, "TX DRAIN trigger failed");

	ret = rx_block_read(dev_i2s, 1);
	zassert_equal(ret, TC_PASS, NULL);
	TC_PRINT("%d<-OK\n", 2);

	/* All but one data block read, stop reception */
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_STOP);
	zassert_equal(ret, 0, "RX STOP trigger failed");

	ret = rx_block_read(dev_i2s, 2);
	zassert_equal(ret, TC_PASS, NULL);
	TC_PRINT("%d<-OK\n", 3);
}

#define TEST_I2S_TRANSFER_RX_OVERRUN_PAUSE_LENGTH_US  200

/** @brief RX buffer overrun.
 *
 * - In case of RX buffer overrun it is possible to read out all RX data blocks
 *   that are stored in the RX queue.
 * - Reading from an empty RX queue when the RX buffer overrun occurred results
 *   in an error.
 * - Sending PREPARE trigger after the RX buffer overrun occurred changes
 *   the interface state to READY.
 */
void test_i2s_transfer_rx_overrun(void)
{
	struct device *dev_i2s;
	void *rx_block;
	size_t rx_size;
	int ret;

	dev_i2s = device_get_binding(I2S_DEV_NAME);
	zassert_not_null(dev_i2s, "device " I2S_DEV_NAME " not found");

	/* Prefill TX queue */
	ret = tx_block_write(dev_i2s, 0, 0);
	zassert_equal(ret, TC_PASS, NULL);

	/* Start reception */
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "RX START trigger failed");

	/* Start transmission */
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "TX START trigger failed");

	for (int i = 0; i < NUM_RX_BLOCKS; i++) {
		ret = tx_block_write(dev_i2s, 0, 0);
		zassert_equal(ret, TC_PASS, NULL);
	}

	/* All data written, flush TX queue and stop the transmission */
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_DRAIN);
	zassert_equal(ret, 0, "TX DRAIN trigger failed");

	/* Wait for transmission to finish */
	k_sleep(TEST_I2S_TRANSFER_RX_OVERRUN_PAUSE_LENGTH_US);

	/* Read all available data blocks in RX queue */
	for (int i = 0; i < NUM_RX_BLOCKS; i++) {
		ret = rx_block_read(dev_i2s, 0);
		zassert_equal(ret, TC_PASS, NULL);
	}

	/* Attempt to read one more data block, expect an error */
	ret = i2s_read(dev_i2s, &rx_block, &rx_size);
	zassert_equal(ret, -EIO, "RX overrun error not detected");

	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_PREPARE);
	zassert_equal(ret, 0, "RX PREPARE trigger failed");

	/* Transmit and receive one more data block */
	ret = tx_block_write(dev_i2s, 0, 0);
	zassert_equal(ret, TC_PASS, NULL);
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "RX START trigger failed");
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "TX START trigger failed");
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_DRAIN);
	zassert_equal(ret, 0, "TX DRAIN trigger failed");
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_STOP);
	zassert_equal(ret, 0, "RX STOP trigger failed");
	ret = rx_block_read(dev_i2s, 0);
	zassert_equal(ret, TC_PASS, NULL);

	k_sleep(200);
}

/** @brief TX buffer underrun.
 *
 * - An attempt to write to the TX queue when TX buffer underrun has occurred
 *   results in an error.
 * - Sending PREPARE trigger after the TX buffer underrun occurred changes
 *   the interface state to READY.
 */
void test_i2s_transfer_tx_underrun(void)
{
	struct device *dev_i2s;
	int ret;

	dev_i2s = device_get_binding(I2S_DEV_NAME);
	zassert_not_null(dev_i2s, "device " I2S_DEV_NAME " not found");

	/* Prefill TX queue */
	ret = tx_block_write(dev_i2s, 0, 0);
	zassert_equal(ret, TC_PASS, NULL);

	/* Start reception */
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "RX START trigger failed");

	/* Start transmission */
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "TX START trigger failed");

	/* Stop reception */
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_STOP);
	zassert_equal(ret, 0, "RX STOP trigger failed");

	ret = rx_block_read(dev_i2s, 0);
	zassert_equal(ret, TC_PASS, NULL);

	k_sleep(200);

	/* Write one more TX data block, expect an error */
	ret = tx_block_write(dev_i2s, 2, -EIO);
	zassert_equal(ret, TC_PASS, NULL);

	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_PREPARE);
	zassert_equal(ret, 0, "TX PREPARE trigger failed");

	/* Transmit and receive two more data blocks */
	ret = tx_block_write(dev_i2s, 1, 0);
	zassert_equal(ret, TC_PASS, NULL);
	ret = tx_block_write(dev_i2s, 1, 0);
	zassert_equal(ret, TC_PASS, NULL);
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "RX START trigger failed");
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_START);
	zassert_equal(ret, 0, "TX START trigger failed");
	ret = rx_block_read(dev_i2s, 1);
	zassert_equal(ret, TC_PASS, NULL);
	ret = i2s_trigger(dev_i2s, I2S_DIR_TX, I2S_TRIGGER_DRAIN);
	zassert_equal(ret, 0, "TX DRAIN trigger failed");
	ret = i2s_trigger(dev_i2s, I2S_DIR_RX, I2S_TRIGGER_STOP);
	zassert_equal(ret, 0, "RX STOP trigger failed");
	ret = rx_block_read(dev_i2s, 1);
	zassert_equal(ret, TC_PASS, NULL);

	k_sleep(200);
}
