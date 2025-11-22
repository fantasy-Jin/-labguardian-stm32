// /**
//  * @File:    flexible_button.h
//  * @Author:  MurphyZhao
//  * @Date:    2018-09-29
//  * 
//  * Copyright (c) 2018-2019 MurphyZhao <d2014zjt@163.com>
//  *               https://github.com/murphyzhao
//  * All rights reserved.
//  * License-Identifier: Apache-2.0
//  *
//  * Licensed under the Apache License, Version 2.0 (the "License"); you may
//  * not use this file except in compliance with the License.
//  * You may obtain a copy of the License at
//  *
//  *    http://www.apache.org/licenses/LICENSE-2.0
//  *
//  * Unless required by applicable law or agreed to in writing, software
//  * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
//  * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  * See the License for the specific language governing permissions and
//  * limitations under the License.
//  *
//  * Change logs:
//  * Date        Author       Notes
//  * 2018-09-29  MurphyZhao   First add
//  * 2019-08-02  MurphyZhao   Migrate code to github.com/murphyzhao account
//  * 2019-12-26  MurphyZhao   Refactor code and implement multiple clicks
//  * 
// */

// #ifndef __BUTTON_H__
// #define __BUTTON_H__

// #include <stdint.h>

// #define FLEX_BTN_SCAN_FREQ_HZ 20 // 1000 / 50 = 20 HZ
// #define FLEX_MS_TO_SCAN_CNT(ms) (ms / 50)

// /* Multiple clicks interval, default 300ms */
// #define MAX_MULTIPLE_CLICKS_INTERVAL (FLEX_MS_TO_SCAN_CNT(300))

// typedef void (*flex_button_response_callback)(void*);

// typedef enum
// {
//     FLEX_BTN_PRESS_DOWN = 0,
//     FLEX_BTN_PRESS_CLICK,
//     FLEX_BTN_PRESS_DOUBLE_CLICK,
//     FLEX_BTN_PRESS_REPEAT_CLICK,
//     FLEX_BTN_PRESS_SHORT_START,
//     FLEX_BTN_PRESS_SHORT_UP,
//     FLEX_BTN_PRESS_LONG_START,
//     FLEX_BTN_PRESS_LONG_UP,
//     FLEX_BTN_PRESS_LONG_HOLD,
//     FLEX_BTN_PRESS_LONG_HOLD_UP,
//     FLEX_BTN_PRESS_MAX,
//     FLEX_BTN_PRESS_NONE,
// } flex_button_event_t;


// typedef struct flex_button
// {
//     struct flex_button* next;

//     uint8_t  (*usr_button_read)(void *);
//     flex_button_response_callback  cb;

//     uint16_t scan_cnt;
//     uint16_t click_cnt;
//     uint16_t max_multiple_clicks_interval;

//     uint16_t debounce_tick; // not used
//     uint16_t short_press_start_tick; // FLEX_MS_TO_SCAN_CNT(1000)
//     uint16_t long_press_start_tick; // FLEX_MS_TO_SCAN_CNT(2000)
//     uint16_t long_hold_start_tick; // FLEX_MS_TO_SCAN_CNT(3000)

//     uint8_t id;
//     uint8_t pressed_logic_level : 1;
//     uint8_t event               : 4;
//     uint8_t status              : 3;
// } flex_button_t;

// #ifdef __cplusplus
// extern "C" {
// #endif

// int32_t flex_button_register(flex_button_t *button);
// flex_button_event_t flex_button_event_read(flex_button_t* button);
// uint8_t flex_button_scan(void);

// #ifdef __cplusplus
// }
// #endif  
// #endif /* __BUTTON_H__ */
