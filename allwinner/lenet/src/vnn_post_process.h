/****************************************************************************
*   Generated by ACUITY 6.0.14
*
*   Neural Network appliction post-process header file
****************************************************************************/
#ifndef _VNN_POST_PROCESS_H_
#define _VNN_POST_PROCESS_H_

vip_status_e save_output_data(
    vip_network_items *network_items);

vip_status_e destroy_network(
    vip_network_items *network_items);

void destroy_network_items(
    vip_network_items *network_items);

#endif
