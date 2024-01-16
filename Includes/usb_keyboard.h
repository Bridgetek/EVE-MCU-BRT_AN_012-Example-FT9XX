/**
 @file keyboard.h
 */
/*
 * ============================================================================
 * History
 * =======
 * 2017-03-15 : Created
 *
 * (C) Copyright Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 * ("Bridgetek") subject to the licence terms set out
 * http://www.ftdichip.com/FTSourceCodeLicenceTerms.htm ("the Licence Terms").
 * You must read the Licence Terms before downloading or using the Software.
 * By installing or using the Software you agree to the Licence Terms. If you
 * do not agree to the Licence Terms then do not download or use the Software.
 *
 * Without prejudice to the Licence Terms, here is a summary of some of the key
 * terms of the Licence Terms (and in the event of any conflict between this
 * summary and the Licence Terms then the text of the Licence Terms will
 * prevail).
 *
 * The Software is provided "as is".
 * There are no warranties (or similar) in relation to the quality of the
 * Software. You use it at your own risk.
 * The Software should not be used in, or for, any medical device, system or
 * appliance. There are exclusions of Bridgetek liability for certain types of loss
 * such as: special loss or damage; incidental loss or damage; indirect or
 * consequential loss or damage; loss of income; loss of business; loss of
 * profits; loss of revenue; loss of contracts; business interruption; loss of
 * the use of money or anticipated savings; loss of information; loss of
 * opportunity; loss of goodwill or reputation; and/or loss of, damage to or
 * corruption of data.
 * There is a monetary cap on Bridgetek's liability.
 * The Software may have subsequently been amended by another user and then
 * distributed by that other user ("Adapted Software").  If so that user may
 * have additional licence terms that apply to those amendments. However, Bridgetek
 * has no liability in relation to those amendments.
 * ============================================================================
 */

#ifndef INCLUDES_USB_KEYBOARD_H_
#define INCLUDES_USB_KEYBOARD_H_

#include <eve_keyboard.h>


/* For MikroC const qualifier will place variables in Flash
 * not just make them constant.
 */
#if defined(__GNUC__)
#define DESCRIPTOR_QUALIFIER const
#elif defined(__MIKROC_PRO_FOR_FT90x__)
#define DESCRIPTOR_QUALIFIER data
#endif

/**
 * @brief Declarations of functions in usb_keyboard.h.
 */
int8_t keyboard_get_report_descriptor(int interface, const uint8_t **descriptor, uint16_t *len);
int8_t keyboard_req_set_idle(int interface, uint8_t idle);
int8_t keyboard_req_get_idle(int interface, uint8_t *idle);
int8_t keyboard_req_set_protocol(int interface, uint8_t protocol);
int8_t keyboard_req_get_protocol(int interface, uint8_t *protocol);
int8_t keyboard_req_get_report(int interface, void *report);
int8_t keyboard_req_set_report(int interface, void *report);
int8_t keyboard_report_enabled(int interface);
void keyboard_set_report_descriptor(int interface, const uint8_t *descriptor, uint16_t len);
void keyboard_report_enable(int interface, int8_t enable);
void keyboard_timer(void);
void keyboard_start(void);
int keyboard_loop(keyboard_in_report_structure_t *keyboard_report_in,
		keyboard_out_report_structure_t *report_buffer_out,
		control_in_report_structure_t *control_report_in);

#endif /* INCLUDES_USB_KEYBOARD_H_ */
