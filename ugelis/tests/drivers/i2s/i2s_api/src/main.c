/*
 * Copyright (c) 2017 comsuisse AG
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @addtogroup t_driver_i2s
 * @{
 * @defgroup t_i2s_basic test_i2s_basic_operations
 * @}
 *
 */

#include <ugelis.h>
#include <ztest.h>

void test_i2s_tx_transfer_configure_0(void);
void test_i2s_rx_transfer_configure_0(void);
void test_i2s_transfer_short(void);
void test_i2s_transfer_long(void);
void test_i2s_rx_sync_start(void);
void test_i2s_rx_empty_timeout(void);
void test_i2s_transfer_restart(void);
void test_i2s_transfer_rx_overrun(void);
void test_i2s_transfer_tx_underrun(void);

void test_i2s_tx_transfer_configure_1(void);
void test_i2s_rx_transfer_configure_1(void);
void test_i2s_state_not_ready_neg(void);
void test_i2s_state_ready_neg(void);
void test_i2s_state_running_neg(void);
void test_i2s_state_stopping_neg(void);
void test_i2s_state_error_neg(void);

void test_main(void)
{
	ztest_test_suite(i2s_loopback_test,
			ztest_unit_test(test_i2s_tx_transfer_configure_0),
			ztest_unit_test(test_i2s_rx_transfer_configure_0),
			ztest_unit_test(test_i2s_transfer_short),
			ztest_unit_test(test_i2s_transfer_long),
			ztest_unit_test(test_i2s_rx_sync_start),
			ztest_unit_test(test_i2s_rx_empty_timeout),
			ztest_unit_test(test_i2s_transfer_restart),
			ztest_unit_test(test_i2s_transfer_rx_overrun),
			ztest_unit_test(test_i2s_transfer_tx_underrun));
	ztest_run_test_suite(i2s_loopback_test);

	ztest_test_suite(i2s_states_test,
			ztest_unit_test(test_i2s_tx_transfer_configure_1),
			ztest_unit_test(test_i2s_rx_transfer_configure_1),
			ztest_unit_test(test_i2s_state_not_ready_neg),
			ztest_unit_test(test_i2s_state_ready_neg),
			ztest_unit_test(test_i2s_state_running_neg),
			ztest_unit_test(test_i2s_state_stopping_neg),
			ztest_unit_test(test_i2s_state_error_neg));
	ztest_run_test_suite(i2s_states_test);
}
