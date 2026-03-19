#include "auto_mode.h"
#include "ui_auto_mode.h"
#include <QMessageBox>
#include "ui_mainwindow.h"
#include "auto_mode.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <QtEndian>
#include <QThread>
#include <QButtonGroup>


uint8_t  m_header1 = 0xA7;   // 8-bit Header
bool     m_isWrite1 = true;  // 1-bit R/W flag
uint16_t m_address1 = 0x0000; // 15-bit Address (using 16-bit type)
uint16_t m_data1 = 0x0000;    // 16-bit Data
uint8_t  m_checksum1 = 0x00;
QByteArray status = 0;


uint16_t on_pul_reg_addr = 0x0108;
uint16_t period_reg_lsb_addr = 0x0109;
uint16_t period_reg_msb_addr = 0x010A;
uint16_t pul_valid_addr = 0x010E;
uint16_t pul_enable_addr = 0x010C;

uint16_t gpo_grp1_addr = 0x0105;
uint16_t gpo_grp2_addr = 0x0106;
uint16_t gpo_grp3_addr = 0x0107;
uint16_t gpo_grp1_en_sts = 0x00;
uint16_t gpo_grp2_en_sts = 0x00;
uint16_t gpo_grp3_en_sts = 0x00;

uint16_t gpi_grp1_live_sts_addr = 0x0100;
uint16_t gpi_grp2_live_sts_addr = 0x0101;
uint16_t gpi_grp3_live_sts_addr = 0x0102;
uint16_t gpi_grp4_live_sts_addr = 0x0103;
uint16_t gpi_grp5_live_sts_addr = 0x0104;
uint16_t gpi_grp6_live_sts_addr = 0x0105;

uint16_t smp_freq = 80; // 80 Mhz
uint16_t req_freq = 1; // 80 Mhz
uint16_t on_time = 0 ;
uint32_t pul_prd_time = (1.0/req_freq)/(1.0/smp_freq);
uint16_t valid_pul_time = (1.0/req_freq)/(1.0/smp_freq);

auto_mode::auto_mode(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::auto_mode)
{
    ui->setupUi(this);
    serial = new QSerialPort(this);
    lvds_rx_group = new QButtonGroup(this);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_1 );
//    lvds_rx_group ->addButton(ui->lvds_rx_en_2 );
//    lvds_rx_group ->addButton(ui->lvds_rx_en_3 );
//    lvds_rx_group ->addButton(ui->lvds_rx_en_4 );
//    lvds_rx_group ->addButton(ui->lvds_rx_en_5 );
//    lvds_rx_group ->addButton(ui->lvds_rx_en_6 );
//    lvds_rx_group ->addButton(ui->lvds_rx_en_7 );
//    lvds_rx_group ->addButton(ui->lvds_rx_en_8 );
//    lvds_rx_group ->addButton(ui->lvds_rx_en_9 );
//    lvds_rx_group ->addButton(ui->lvds_rx_en_10);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_11);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_12);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_13);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_14);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_15);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_16);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_17);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_18);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_19);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_20);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_21);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_22);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_23);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_24);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_25);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_26);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_27);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_28);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_29);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_30);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_31);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_32);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_33);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_34);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_35);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_36);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_37);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_38);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_39);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_40);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_41);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_42);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_43);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_44);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_45);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_46);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_47);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_48);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_49);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_50);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_51);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_52);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_53);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_54);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_55);
//    lvds_rx_group ->addButton(ui->lvds_rx_en_56);

    lvds_cn_rx1_group = new QButtonGroup(this);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_35);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_52);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_26);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_31);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_25);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_30);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_49);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_46);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_28);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_54);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_50);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_40);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_48);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_45);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_53);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_41);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_38);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_43);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_6);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_44);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_8);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_11);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_7);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_16);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_20);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_14);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_17);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_18);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_1);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_3);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_10);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_4);
    lvds_cn_rx1_group ->addButton(ui->lvds_rx_en_19);
    lvds_cn_rx1_group ->setExclusive(false);

    lvds_cn_rx2_group = new QButtonGroup(this);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_33);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_29);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_27);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_34);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_37);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_47);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_39);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_55);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_42);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_51);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_36);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_24);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_15);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_32);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_9);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_2);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_23);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_21);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_12);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_56);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_22);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_13);
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_5);
    lvds_cn_rx2_group ->setExclusive(false);

    lvds_cn_rx2_tx_dis_group = new QButtonGroup(this);
    lvds_cn_rx2_tx_dis_group ->addButton(ui->lvds_tx_en_1);
    lvds_cn_rx2_tx_dis_group ->addButton(ui->lvds_tx_en_5);
    lvds_cn_rx2_tx_dis_group ->addButton(ui->lvds_tx_en_6);
    lvds_cn_rx2_tx_dis_group ->addButton(ui->lvds_tx_en_9);
    lvds_cn_rx2_tx_dis_group ->addButton(ui->lvds_tx_en_18);
    lvds_cn_rx2_tx_dis_group ->addButton(ui->lvds_tx_en_19);
    lvds_cn_rx2_tx_dis_group ->addButton(ui->lvds_tx_en_21);
    lvds_cn_rx2_tx_dis_group ->addButton(ui->lvds_tx_en_22);
    lvds_cn_rx2_tx_dis_group ->addButton(ui->lvds_tx_en_26);
    lvds_cn_rx2_tx_dis_group ->addButton(ui->lvds_tx_en_29);
    lvds_cn_rx2_tx_dis_group ->setExclusive(false);

    lvds_cn_rx2_tx_group = new QButtonGroup(this);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_2);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_3);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_4);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_7);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_8);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_10);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_11);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_12);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_13);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_14);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_15);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_16);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_17);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_20);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_23);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_24);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_25);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_27);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_28);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_30);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_31);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_32);
    lvds_cn_rx2_tx_group ->addButton(ui->lvds_tx_en_33);
    lvds_cn_rx2_tx_group ->setExclusive(false);


    rs422_rx_group = new QButtonGroup(this);
    rs422_rx_group ->addButton(ui->rs422_rx_en_57);
    rs422_rx_group ->addButton(ui->rs422_rx_en_58);
    rs422_rx_group ->addButton(ui->rs422_rx_en_59);
    rs422_rx_group ->addButton(ui->rs422_rx_en_60);
    rs422_rx_group ->setExclusive(false);

    lvttl_rx_group = new QButtonGroup(this);
    lvttl_rx_group ->addButton(ui->LVTTL_rx_en_61);
    lvttl_rx_group ->addButton(ui->LVTTL_rx_en_62);
    lvttl_rx_group ->addButton(ui->LVTTL_rx_en_63);
    lvttl_rx_group ->addButton(ui->LVTTL_rx_en_64);
    lvttl_rx_group ->addButton(ui->LVTTL_rx_en_65);
    lvttl_rx_group ->addButton(ui->LVTTL_rx_en_66);
    lvttl_rx_group ->addButton(ui->LVTTL_rx_en_67);
    lvttl_rx_group ->addButton(ui->LVTTL_rx_en_68);
    lvttl_rx_group ->setExclusive(false);
    rx_lvds_group << lvds_cn_rx1_group << lvds_cn_rx2_group;
    rx_group << lvds_cn_rx1_group << lvds_cn_rx2_group << rs422_rx_group << lvttl_rx_group;


    lvds_freq_group << ui->lvds_freq_35
                    << ui->lvds_freq_52
                    << ui->lvds_freq_26
                    << ui->lvds_freq_31
                    << ui->lvds_freq_25
                    << ui->lvds_freq_30
                    << ui->lvds_freq_49
                    << ui->lvds_freq_46
                    << ui->lvds_freq_28
                    << ui->lvds_freq_54
                    << ui->lvds_freq_50
                    << ui->lvds_freq_40
                    << ui->lvds_freq_48
                    << ui->lvds_freq_45
                    << ui->lvds_freq_53
                    << ui->lvds_freq_41
                    << ui->lvds_freq_38
                    << ui->lvds_freq_43
                    << ui->lvds_freq_6
                    << ui->lvds_freq_44
                    << ui->lvds_freq_8
                    << ui->lvds_freq_11
                    << ui->lvds_freq_7
                    << ui->lvds_freq_16
                    << ui->lvds_freq_20
                    << ui->lvds_freq_14
                    << ui->lvds_freq_17
                    << ui->lvds_freq_18
                    << ui->lvds_freq_1
                    << ui->lvds_freq_3
                    << ui->lvds_freq_10
                    << ui->lvds_freq_4
                    << ui->lvds_freq_19
                    << ui->lvds_freq_56
                    << ui->lvds_freq_33
                    << ui->lvds_freq_29
                    << ui->lvds_freq_27
                    << ui->lvds_freq_34
                    << ui->lvds_freq_37
                    << ui->lvds_freq_47
                    << ui->lvds_freq_39
                    << ui->lvds_freq_55
                    << ui->lvds_freq_42
                    << ui->lvds_freq_51
                    << ui->lvds_freq_36
                    << ui->lvds_freq_24
                    << ui->lvds_freq_15
                    << ui->lvds_freq_32
                    << ui->lvds_freq_9
                    << ui->lvds_freq_2
                    << ui->lvds_freq_23
                    << ui->lvds_freq_21
                    << ui->lvds_freq_12
                    << ui->lvds_freq_22
                    << ui->lvds_freq_13
                    << ui->lvds_freq_5;

    lvds_glitch_group << ui->lvds_glitch_cnt_35
                      << ui->lvds_glitch_cnt_52
                      << ui->lvds_glitch_cnt_26
                      << ui->lvds_glitch_cnt_31
                      << ui->lvds_glitch_cnt_25
                      << ui->lvds_glitch_cnt_30
                      << ui->lvds_glitch_cnt_49
                      << ui->lvds_glitch_cnt_46
                      << ui->lvds_glitch_cnt_28
                      << ui->lvds_glitch_cnt_54
                      << ui->lvds_glitch_cnt_50
                      << ui->lvds_glitch_cnt_40
                      << ui->lvds_glitch_cnt_48
                      << ui->lvds_glitch_cnt_45
                      << ui->lvds_glitch_cnt_53
                      << ui->lvds_glitch_cnt_41
                      << ui->lvds_glitch_cnt_38
                      << ui->lvds_glitch_cnt_43
                      << ui->lvds_glitch_cnt_6
                      << ui->lvds_glitch_cnt_44
                      << ui->lvds_glitch_cnt_8
                      << ui->lvds_glitch_cnt_11
                      << ui->lvds_glitch_cnt_7
                      << ui->lvds_glitch_cnt_16
                      << ui->lvds_glitch_cnt_20
                      << ui->lvds_glitch_cnt_14
                      << ui->lvds_glitch_cnt_17
                      << ui->lvds_glitch_cnt_18
                      << ui->lvds_glitch_cnt_1
                      << ui->lvds_glitch_cnt_3
                      << ui->lvds_glitch_cnt_10
                      << ui->lvds_glitch_cnt_4
                      << ui->lvds_glitch_cnt_19
                      << ui->lvds_glitch_cnt_56
                      << ui->lvds_glitch_cnt_33
                      << ui->lvds_glitch_cnt_29
                      << ui->lvds_glitch_cnt_27
                      << ui->lvds_glitch_cnt_34
                      << ui->lvds_glitch_cnt_37
                      << ui->lvds_glitch_cnt_47
                      << ui->lvds_glitch_cnt_39
                      << ui->lvds_glitch_cnt_55
                      << ui->lvds_glitch_cnt_42
                      << ui->lvds_glitch_cnt_51
                      << ui->lvds_glitch_cnt_36
                      << ui->lvds_glitch_cnt_24
                      << ui->lvds_glitch_cnt_15
                      << ui->lvds_glitch_cnt_32
                      << ui->lvds_glitch_cnt_9
                      << ui->lvds_glitch_cnt_2
                      << ui->lvds_glitch_cnt_23
                      << ui->lvds_glitch_cnt_21
                      << ui->lvds_glitch_cnt_12
                      << ui->lvds_glitch_cnt_22
                      << ui->lvds_glitch_cnt_13
                      << ui->lvds_glitch_cnt_5;

    rs422_freq_group << ui->RS422_freq_57
                    << ui->RS422_freq_58
                    << ui->RS422_freq_59
                    << ui->RS422_freq_60;

    rs422_glitch_group << ui->RS422_glitch_cnt_57
                    << ui->RS422_glitch_cnt_58
                    << ui->RS422_glitch_cnt_59
                    << ui->RS422_glitch_cnt_60;

    lvttl_freq_group << ui->rx_freq_61
                     << ui->rx_freq_62
                     << ui->rx_freq_63
                     << ui->rx_freq_64
                     << ui->rx_freq_65
                     << ui->rx_freq_66
                     << ui->rx_freq_67
                     << ui->rx_freq_68;

    lvttl_glitch_group << ui->glitch_cnt_61
                     << ui->glitch_cnt_62
                     << ui->glitch_cnt_63
                     << ui->glitch_cnt_64
                     << ui->glitch_cnt_65
                     << ui->glitch_cnt_66
                     << ui->glitch_cnt_67
                     << ui->glitch_cnt_68;

    gpi_grp1_pul_cnt_addr_lst.assign(16,0x0000);
    gpi_grp1_pul_cnt_addr_lst[0 ] = 0x110;
    gpi_grp1_pul_cnt_addr_lst[1 ] = 0x111;
    gpi_grp1_pul_cnt_addr_lst[2 ] = 0x112;
    gpi_grp1_pul_cnt_addr_lst[3 ] = 0x113;
    gpi_grp1_pul_cnt_addr_lst[4 ] = 0x114;
    gpi_grp1_pul_cnt_addr_lst[5 ] = 0x115;
    gpi_grp1_pul_cnt_addr_lst[6 ] = 0x116;
    gpi_grp1_pul_cnt_addr_lst[7 ] = 0x117;
    gpi_grp1_pul_cnt_addr_lst[8 ] = 0x118;
    gpi_grp1_pul_cnt_addr_lst[9 ] = 0x119;
    gpi_grp1_pul_cnt_addr_lst[10] = 0x11A;
    gpi_grp1_pul_cnt_addr_lst[11] = 0x11B;
    gpi_grp1_pul_cnt_addr_lst[12] = 0x11C;
    gpi_grp1_pul_cnt_addr_lst[13] = 0x11D;
    gpi_grp1_pul_cnt_addr_lst[14] = 0x11E;
    gpi_grp1_pul_cnt_addr_lst[15] = 0x11F;

    gpi_grp2_pul_cnt_addr_lst.assign(16,0x0000);
    gpi_grp2_pul_cnt_addr_lst[0 ] = 0x120;
    gpi_grp2_pul_cnt_addr_lst[1 ] = 0x121;
    gpi_grp2_pul_cnt_addr_lst[2 ] = 0x122;
    gpi_grp2_pul_cnt_addr_lst[3 ] = 0x123;
    gpi_grp2_pul_cnt_addr_lst[4 ] = 0x124;
    gpi_grp2_pul_cnt_addr_lst[5 ] = 0x125;
    gpi_grp2_pul_cnt_addr_lst[6 ] = 0x126;
    gpi_grp2_pul_cnt_addr_lst[7 ] = 0x127;
    gpi_grp2_pul_cnt_addr_lst[8 ] = 0x128;
    gpi_grp2_pul_cnt_addr_lst[9 ] = 0x129;
    gpi_grp2_pul_cnt_addr_lst[10] = 0x12A;
    gpi_grp2_pul_cnt_addr_lst[11] = 0x12B;
    gpi_grp2_pul_cnt_addr_lst[12] = 0x12C;
    gpi_grp2_pul_cnt_addr_lst[13] = 0x12D;
    gpi_grp2_pul_cnt_addr_lst[14] = 0x12E;
    gpi_grp2_pul_cnt_addr_lst[15] = 0x12F;


    gpi_grp3_pul_cnt_addr_lst.assign(16,0x0000);
    gpi_grp3_pul_cnt_addr_lst[0 ] = 0x130;
    gpi_grp3_pul_cnt_addr_lst[1 ] = 0x131;
    gpi_grp3_pul_cnt_addr_lst[2 ] = 0x132;
    gpi_grp3_pul_cnt_addr_lst[3 ] = 0x133;
    gpi_grp3_pul_cnt_addr_lst[4 ] = 0x134;
    gpi_grp3_pul_cnt_addr_lst[5 ] = 0x135;
    gpi_grp3_pul_cnt_addr_lst[6 ] = 0x136;
    gpi_grp3_pul_cnt_addr_lst[7 ] = 0x137;
    gpi_grp3_pul_cnt_addr_lst[8 ] = 0x138;
    gpi_grp3_pul_cnt_addr_lst[9 ] = 0x139;
    gpi_grp3_pul_cnt_addr_lst[10] = 0x13A;
    gpi_grp3_pul_cnt_addr_lst[11] = 0x13B;
    gpi_grp3_pul_cnt_addr_lst[12] = 0x13C;
    gpi_grp3_pul_cnt_addr_lst[13] = 0x13D;
    gpi_grp3_pul_cnt_addr_lst[14] = 0x13E;
    gpi_grp3_pul_cnt_addr_lst[15] = 0x13F;


    gpi_grp4_pul_cnt_addr_lst.assign(16,0x0000);
    gpi_grp4_pul_cnt_addr_lst[0 ] = 0x140;
    gpi_grp4_pul_cnt_addr_lst[1 ] = 0x141;
    gpi_grp4_pul_cnt_addr_lst[2 ] = 0x142;
    gpi_grp4_pul_cnt_addr_lst[3 ] = 0x143;
    gpi_grp4_pul_cnt_addr_lst[4 ] = 0x144;
    gpi_grp4_pul_cnt_addr_lst[5 ] = 0x145;
    gpi_grp4_pul_cnt_addr_lst[6 ] = 0x146;
    gpi_grp4_pul_cnt_addr_lst[7 ] = 0x147;
    gpi_grp4_pul_cnt_addr_lst[8 ] = 0x148;
    gpi_grp4_pul_cnt_addr_lst[9 ] = 0x149;
    gpi_grp4_pul_cnt_addr_lst[10] = 0x14A;
    gpi_grp4_pul_cnt_addr_lst[11] = 0x14B;
    gpi_grp4_pul_cnt_addr_lst[12] = 0x14C;
    gpi_grp4_pul_cnt_addr_lst[13] = 0x14D;
    gpi_grp4_pul_cnt_addr_lst[14] = 0x14E;
    gpi_grp4_pul_cnt_addr_lst[15] = 0x14F;


    gpi_grp5_pul_cnt_addr_lst.assign(4,0x0000);
    gpi_grp5_pul_cnt_addr_lst[0]= 0x150;
    gpi_grp5_pul_cnt_addr_lst[1]= 0x151;
    gpi_grp5_pul_cnt_addr_lst[2]= 0x152;
    gpi_grp5_pul_cnt_addr_lst[3]= 0x153;
//    gpi_grp5_pul_cnt_addr_lst[11]= 0x153;
//    gpi_grp5_pul_cnt_addr_lst[10]= 0x154;
//    gpi_grp5_pul_cnt_addr_lst[9 ]= 0x155;
//    gpi_grp5_pul_cnt_addr_lst[8 ]= 0x156;
//    gpi_grp5_pul_cnt_addr_lst[7 ]= 0x157;
//    gpi_grp5_pul_cnt_addr_lst[6 ]= 0x158;
//    gpi_grp5_pul_cnt_addr_lst[5 ] = 0x15A;
//    gpi_grp5_pul_cnt_addr_lst[4 ] = 0x15B;
//    gpi_grp5_pul_cnt_addr_lst[3 ] = 0x15C;
//    gpi_grp5_pul_cnt_addr_lst[2 ] = 0x15D;
//    gpi_grp5_pul_cnt_addr_lst[1 ] = 0x15E;
//    gpi_grp5_pul_cnt_addr_lst[0 ] = 0x15F;
    lvds_rx_index.assign(56,0x000);
    lvds_rx_index[0] = 	34 ;
    lvds_rx_index[1] = 	51 ;
    lvds_rx_index[2] = 	25 ;
    lvds_rx_index[3] = 	30 ;
    lvds_rx_index[4] = 	24 ;
    lvds_rx_index[5] = 	29 ;
    lvds_rx_index[6] = 	48 ;
    lvds_rx_index[7] = 	45 ;
    lvds_rx_index[8] = 	27 ;
    lvds_rx_index[9] = 	53 ;
    lvds_rx_index[10] = 49;
    lvds_rx_index[11] = 39;
    lvds_rx_index[12] = 47;
    lvds_rx_index[13] = 44;
    lvds_rx_index[14] = 52;
    lvds_rx_index[15] = 40;
    lvds_rx_index[16] = 37;
    lvds_rx_index[17] = 42;
    lvds_rx_index[18] = 5;
    lvds_rx_index[19] = 43;
    lvds_rx_index[20] = 7;
    lvds_rx_index[21] = 10;
    lvds_rx_index[22] = 6;
    lvds_rx_index[23] = 15;
    lvds_rx_index[24] = 19;
    lvds_rx_index[25] = 13;
    lvds_rx_index[26] = 16;
    lvds_rx_index[27] = 17;
    lvds_rx_index[28] = 0;
    lvds_rx_index[29] = 2;
    lvds_rx_index[30] = 9;
    lvds_rx_index[31] = 3;
    lvds_rx_index[32] = 18;
    lvds_rx_index[33] = 55;
    lvds_rx_index[34] = 32;
    lvds_rx_index[35] = 28;
    lvds_rx_index[36] = 26;
    lvds_rx_index[37] = 33;
    lvds_rx_index[38] = 36;
    lvds_rx_index[39] = 46;
    lvds_rx_index[40] = 38;
    lvds_rx_index[41] = 54;
    lvds_rx_index[42] = 41;
    lvds_rx_index[43] = 50;
    lvds_rx_index[44] = 35;
    lvds_rx_index[45] = 23;
    lvds_rx_index[46] = 14;
    lvds_rx_index[47] = 31;
    lvds_rx_index[48] = 8;
    lvds_rx_index[49] = 1;
    lvds_rx_index[50] = 22;
    lvds_rx_index[51] = 20;
    lvds_rx_index[52] = 11;
    lvds_rx_index[53] = 21;
    lvds_rx_index[54] = 12;
    lvds_rx_index[55] = 4;

    gpi_grp1_glitch_cnt_addr_lst.assign(16,0x0000);
    gpi_grp1_glitch_cnt_addr_lst[0 ] = 0x160;
    gpi_grp1_glitch_cnt_addr_lst[1 ] = 0x161;
    gpi_grp1_glitch_cnt_addr_lst[2 ] = 0x162;
    gpi_grp1_glitch_cnt_addr_lst[3 ] = 0x163;
    gpi_grp1_glitch_cnt_addr_lst[4 ] = 0x164;
    gpi_grp1_glitch_cnt_addr_lst[5 ] = 0x165;
    gpi_grp1_glitch_cnt_addr_lst[6 ] = 0x166;
    gpi_grp1_glitch_cnt_addr_lst[7 ] = 0x167;
    gpi_grp1_glitch_cnt_addr_lst[8 ] = 0x168;
    gpi_grp1_glitch_cnt_addr_lst[9 ] = 0x169;
    gpi_grp1_glitch_cnt_addr_lst[10] = 0x16A;
    gpi_grp1_glitch_cnt_addr_lst[11] = 0x16B;
    gpi_grp1_glitch_cnt_addr_lst[12] = 0x16C;
    gpi_grp1_glitch_cnt_addr_lst[13] = 0x16D;
    gpi_grp1_glitch_cnt_addr_lst[14] = 0x16E;
    gpi_grp1_glitch_cnt_addr_lst[15] = 0x16F;

    gpi_grp2_glitch_cnt_addr_lst.assign(16,0x0000);
    gpi_grp2_glitch_cnt_addr_lst[0 ]=   0x170;
    gpi_grp2_glitch_cnt_addr_lst[1 ]=   0x171;
    gpi_grp2_glitch_cnt_addr_lst[2 ]=   0x172;
    gpi_grp2_glitch_cnt_addr_lst[3 ]=   0x173;
    gpi_grp2_glitch_cnt_addr_lst[4 ]=   0x174;
    gpi_grp2_glitch_cnt_addr_lst[5 ]=   0x175;
    gpi_grp2_glitch_cnt_addr_lst[6 ]=   0x176;
    gpi_grp2_glitch_cnt_addr_lst[7 ]=   0x177;
    gpi_grp2_glitch_cnt_addr_lst[8 ]=   0x178;
    gpi_grp2_glitch_cnt_addr_lst[9 ]=   0x179;
    gpi_grp2_glitch_cnt_addr_lst[10] =  0x17A;
    gpi_grp2_glitch_cnt_addr_lst[11] =  0x17B;
    gpi_grp2_glitch_cnt_addr_lst[12] =  0x17C;
    gpi_grp2_glitch_cnt_addr_lst[13] =  0x17D;
    gpi_grp2_glitch_cnt_addr_lst[14] =  0x17E;
    gpi_grp2_glitch_cnt_addr_lst[15] =  0x17F;

    gpi_grp3_glitch_cnt_addr_lst.assign(16,0x0000);
    gpi_grp3_glitch_cnt_addr_lst[0 ]=   0x180;
    gpi_grp3_glitch_cnt_addr_lst[1 ]=   0x181;
    gpi_grp3_glitch_cnt_addr_lst[2 ]=   0x182;
    gpi_grp3_glitch_cnt_addr_lst[3 ]=   0x183;
    gpi_grp3_glitch_cnt_addr_lst[4 ]=   0x184;
    gpi_grp3_glitch_cnt_addr_lst[5 ]=   0x185;
    gpi_grp3_glitch_cnt_addr_lst[6 ]=   0x186;
    gpi_grp3_glitch_cnt_addr_lst[7 ]=   0x187;
    gpi_grp3_glitch_cnt_addr_lst[8 ]=   0x188;
    gpi_grp3_glitch_cnt_addr_lst[9 ]=   0x189;
    gpi_grp3_glitch_cnt_addr_lst[10] =  0x18A;
    gpi_grp3_glitch_cnt_addr_lst[11] =  0x18B;
    gpi_grp3_glitch_cnt_addr_lst[12] =  0x18C;
    gpi_grp3_glitch_cnt_addr_lst[13] =  0x18D;
    gpi_grp3_glitch_cnt_addr_lst[14] =  0x18E;
    gpi_grp3_glitch_cnt_addr_lst[15] =  0x18F;


    gpi_grp4_glitch_cnt_addr_lst.assign(16,0x0000);
    gpi_grp4_glitch_cnt_addr_lst[0 ]=   0x190;
    gpi_grp4_glitch_cnt_addr_lst[1 ]=   0x191;
    gpi_grp4_glitch_cnt_addr_lst[2 ]=   0x192;
    gpi_grp4_glitch_cnt_addr_lst[3 ]=   0x193;
    gpi_grp4_glitch_cnt_addr_lst[4 ]=   0x194;
    gpi_grp4_glitch_cnt_addr_lst[5 ]=   0x195;
    gpi_grp4_glitch_cnt_addr_lst[6 ]=   0x196;
    gpi_grp4_glitch_cnt_addr_lst[7 ]=   0x197;
    gpi_grp4_glitch_cnt_addr_lst[8 ]=   0x198;
    gpi_grp4_glitch_cnt_addr_lst[9 ]=   0x199;
    gpi_grp4_glitch_cnt_addr_lst[10] =  0x19A;
    gpi_grp4_glitch_cnt_addr_lst[11] =  0x19B;
    gpi_grp4_glitch_cnt_addr_lst[12] =  0x19C;
    gpi_grp4_glitch_cnt_addr_lst[13] =  0x19D;
    gpi_grp4_glitch_cnt_addr_lst[14] =  0x19E;
    gpi_grp4_glitch_cnt_addr_lst[15] =  0x19F;

    gpi_grp5_glitch_cnt_addr_lst.assign(16,0x0000);
    gpi_grp5_glitch_cnt_addr_lst[0]=   0x1A0;
    gpi_grp5_glitch_cnt_addr_lst[1]=   0x1A1;
    gpi_grp5_glitch_cnt_addr_lst[2]=   0x1A2;
    gpi_grp5_glitch_cnt_addr_lst[3]=   0x1A2;
//    gpi_grp5_glitch_cnt_addr_lst[4]=   0x1a3;
//    gpi_grp5_glitch_cnt_addr_lst[5]=   0x1a4;
//    gpi_grp5_glitch_cnt_addr_lst[6]=   0x1a5;
//    gpi_grp5_glitch_cnt_addr_lst[8 ]=   0x1a6;
//    gpi_grp5_glitch_cnt_addr_lst[7 ]=   0x1a7;
//    gpi_grp5_glitch_cnt_addr_lst[6 ]=   0x1a8;
//    gpi_grp5_glitch_cnt_addr_lst[5 ] =  0x1aA;
//    gpi_grp5_glitch_cnt_addr_lst[4 ] =  0x1aB;
//    gpi_grp5_glitch_cnt_addr_lst[3 ] =  0x1aC;
//    gpi_grp5_glitch_cnt_addr_lst[2 ] =  0x1aD;
//    gpi_grp5_glitch_cnt_addr_lst[1 ] =  0x1aE;
//    gpi_grp5_glitch_cnt_addr_lst[0 ] =  0x1aF;

    lvds_grp_glitch_cnt_addr_lst.assign(56,0x0000);
    lvds_grp_glitch_cnt_addr_lst[0]  =	 gpi_grp1_glitch_cnt_addr_lst[15];
    lvds_grp_glitch_cnt_addr_lst[1]  =   gpi_grp1_glitch_cnt_addr_lst[14];
    lvds_grp_glitch_cnt_addr_lst[2]  =   gpi_grp1_glitch_cnt_addr_lst[13];
    lvds_grp_glitch_cnt_addr_lst[3]  =   gpi_grp1_glitch_cnt_addr_lst[12];
    lvds_grp_glitch_cnt_addr_lst[4]  =   gpi_grp1_glitch_cnt_addr_lst[11];
    lvds_grp_glitch_cnt_addr_lst[5]  =   gpi_grp1_glitch_cnt_addr_lst[10];
    lvds_grp_glitch_cnt_addr_lst[6]  =   gpi_grp1_glitch_cnt_addr_lst[9 ];
    lvds_grp_glitch_cnt_addr_lst[7]  =   gpi_grp1_glitch_cnt_addr_lst[8 ];
    lvds_grp_glitch_cnt_addr_lst[8]  =   gpi_grp1_glitch_cnt_addr_lst[7 ];
    lvds_grp_glitch_cnt_addr_lst[9]  =   gpi_grp1_glitch_cnt_addr_lst[6 ];
    lvds_grp_glitch_cnt_addr_lst[10] =   gpi_grp1_glitch_cnt_addr_lst[5 ];
    lvds_grp_glitch_cnt_addr_lst[11] =   gpi_grp1_glitch_cnt_addr_lst[4 ];
    lvds_grp_glitch_cnt_addr_lst[12] =   gpi_grp1_glitch_cnt_addr_lst[3 ];
    lvds_grp_glitch_cnt_addr_lst[13] =   gpi_grp1_glitch_cnt_addr_lst[2 ];
    lvds_grp_glitch_cnt_addr_lst[14] =   gpi_grp1_glitch_cnt_addr_lst[1 ];
    lvds_grp_glitch_cnt_addr_lst[15] =   gpi_grp1_glitch_cnt_addr_lst[0 ];
    lvds_grp_glitch_cnt_addr_lst[16] =   gpi_grp2_glitch_cnt_addr_lst[15];
    lvds_grp_glitch_cnt_addr_lst[17] =   gpi_grp2_glitch_cnt_addr_lst[14];
    lvds_grp_glitch_cnt_addr_lst[18] =   gpi_grp2_glitch_cnt_addr_lst[13];
    lvds_grp_glitch_cnt_addr_lst[19] =   gpi_grp2_glitch_cnt_addr_lst[12];
    lvds_grp_glitch_cnt_addr_lst[20] =   gpi_grp2_glitch_cnt_addr_lst[11];
    lvds_grp_glitch_cnt_addr_lst[21] =   gpi_grp2_glitch_cnt_addr_lst[10];
    lvds_grp_glitch_cnt_addr_lst[22] =   gpi_grp2_glitch_cnt_addr_lst[9 ];
    lvds_grp_glitch_cnt_addr_lst[23] =   gpi_grp2_glitch_cnt_addr_lst[8 ];
    lvds_grp_glitch_cnt_addr_lst[24] =   gpi_grp2_glitch_cnt_addr_lst[7 ];
    lvds_grp_glitch_cnt_addr_lst[25] =   gpi_grp2_glitch_cnt_addr_lst[6 ];
    lvds_grp_glitch_cnt_addr_lst[26] =   gpi_grp2_glitch_cnt_addr_lst[5 ];
    lvds_grp_glitch_cnt_addr_lst[27] =   gpi_grp2_glitch_cnt_addr_lst[4 ];
    lvds_grp_glitch_cnt_addr_lst[28] =   gpi_grp2_glitch_cnt_addr_lst[3 ];
    lvds_grp_glitch_cnt_addr_lst[29] =   gpi_grp2_glitch_cnt_addr_lst[2 ];
    lvds_grp_glitch_cnt_addr_lst[30] =   gpi_grp2_glitch_cnt_addr_lst[1 ];
    lvds_grp_glitch_cnt_addr_lst[31] =   gpi_grp2_glitch_cnt_addr_lst[0 ];
    lvds_grp_glitch_cnt_addr_lst[32] =   gpi_grp3_glitch_cnt_addr_lst[15];
    lvds_grp_glitch_cnt_addr_lst[33] =   gpi_grp3_glitch_cnt_addr_lst[14];
    lvds_grp_glitch_cnt_addr_lst[34] =   gpi_grp3_glitch_cnt_addr_lst[13];
    lvds_grp_glitch_cnt_addr_lst[35] =   gpi_grp3_glitch_cnt_addr_lst[12];
    lvds_grp_glitch_cnt_addr_lst[36] =   gpi_grp3_glitch_cnt_addr_lst[11];
    lvds_grp_glitch_cnt_addr_lst[37] =   gpi_grp3_glitch_cnt_addr_lst[10];
    lvds_grp_glitch_cnt_addr_lst[38] =   gpi_grp3_glitch_cnt_addr_lst[9 ];
    lvds_grp_glitch_cnt_addr_lst[39] =   gpi_grp3_glitch_cnt_addr_lst[8 ];
    lvds_grp_glitch_cnt_addr_lst[40] =   gpi_grp3_glitch_cnt_addr_lst[7 ];
    lvds_grp_glitch_cnt_addr_lst[41] =   gpi_grp3_glitch_cnt_addr_lst[6 ];
    lvds_grp_glitch_cnt_addr_lst[42] =   gpi_grp3_glitch_cnt_addr_lst[5 ];
    lvds_grp_glitch_cnt_addr_lst[43] =   gpi_grp3_glitch_cnt_addr_lst[4 ];
    lvds_grp_glitch_cnt_addr_lst[44] =   gpi_grp3_glitch_cnt_addr_lst[3 ];
    lvds_grp_glitch_cnt_addr_lst[45] =   gpi_grp3_glitch_cnt_addr_lst[2 ];
    lvds_grp_glitch_cnt_addr_lst[46] =   gpi_grp3_glitch_cnt_addr_lst[1 ];
    lvds_grp_glitch_cnt_addr_lst[47] =   gpi_grp3_glitch_cnt_addr_lst[0 ];
    lvds_grp_glitch_cnt_addr_lst[48] =   gpi_grp4_glitch_cnt_addr_lst[15];
    lvds_grp_glitch_cnt_addr_lst[49] =   gpi_grp4_glitch_cnt_addr_lst[14];
    lvds_grp_glitch_cnt_addr_lst[50] =   gpi_grp4_glitch_cnt_addr_lst[13];
    lvds_grp_glitch_cnt_addr_lst[51] =   gpi_grp4_glitch_cnt_addr_lst[12];
    lvds_grp_glitch_cnt_addr_lst[52] =   gpi_grp4_glitch_cnt_addr_lst[11];
    lvds_grp_glitch_cnt_addr_lst[53] =   gpi_grp4_glitch_cnt_addr_lst[10];
    lvds_grp_glitch_cnt_addr_lst[54] =   gpi_grp4_glitch_cnt_addr_lst[9 ];
    lvds_grp_glitch_cnt_addr_lst[55] =   gpi_grp5_glitch_cnt_addr_lst[3 ];

    rs422_grp_glitch_cnt_addr_lst.assign(4,0x0000);
    rs422_grp_glitch_cnt_addr_lst[0] = gpi_grp4_glitch_cnt_addr_lst[8];
    rs422_grp_glitch_cnt_addr_lst[1] = gpi_grp4_glitch_cnt_addr_lst[7];
    rs422_grp_glitch_cnt_addr_lst[2] = gpi_grp4_glitch_cnt_addr_lst[6];
    rs422_grp_glitch_cnt_addr_lst[3] = gpi_grp4_glitch_cnt_addr_lst[5];

    lvttl_grp_glitch_cnt_addr_lst.assign(8,0x00);
    lvttl_grp_glitch_cnt_addr_lst[0] = gpi_grp4_glitch_cnt_addr_lst[4];
    lvttl_grp_glitch_cnt_addr_lst[1] = gpi_grp4_glitch_cnt_addr_lst[3];
    lvttl_grp_glitch_cnt_addr_lst[2] = gpi_grp4_glitch_cnt_addr_lst[2];
    lvttl_grp_glitch_cnt_addr_lst[3] = gpi_grp4_glitch_cnt_addr_lst[1];
    lvttl_grp_glitch_cnt_addr_lst[4] = gpi_grp4_glitch_cnt_addr_lst[0];
    lvttl_grp_glitch_cnt_addr_lst[5] = gpi_grp5_glitch_cnt_addr_lst[2];
    lvttl_grp_glitch_cnt_addr_lst[6] = gpi_grp5_glitch_cnt_addr_lst[1];
    lvttl_grp_glitch_cnt_addr_lst[7] = gpi_grp5_glitch_cnt_addr_lst[0];



    lvds_grp_pul_cnt_addr_lst.assign(56,0x0000);
    lvds_grp_pul_cnt_addr_lst[0]  =	  gpi_grp1_pul_cnt_addr_lst[15];
    lvds_grp_pul_cnt_addr_lst[1]  =   gpi_grp1_pul_cnt_addr_lst[14];
    lvds_grp_pul_cnt_addr_lst[2]  =   gpi_grp1_pul_cnt_addr_lst[13];
    lvds_grp_pul_cnt_addr_lst[3]  =   gpi_grp1_pul_cnt_addr_lst[12];
    lvds_grp_pul_cnt_addr_lst[4]  =   gpi_grp1_pul_cnt_addr_lst[11];
    lvds_grp_pul_cnt_addr_lst[5]  =   gpi_grp1_pul_cnt_addr_lst[10];
    lvds_grp_pul_cnt_addr_lst[6]  =   gpi_grp1_pul_cnt_addr_lst[9 ];
    lvds_grp_pul_cnt_addr_lst[7]  =   gpi_grp1_pul_cnt_addr_lst[8 ];
    lvds_grp_pul_cnt_addr_lst[8]  =   gpi_grp1_pul_cnt_addr_lst[7 ];
    lvds_grp_pul_cnt_addr_lst[9]  =   gpi_grp1_pul_cnt_addr_lst[6 ];
    lvds_grp_pul_cnt_addr_lst[10] =   gpi_grp1_pul_cnt_addr_lst[5 ];
    lvds_grp_pul_cnt_addr_lst[11] =   gpi_grp1_pul_cnt_addr_lst[4 ];
    lvds_grp_pul_cnt_addr_lst[12] =   gpi_grp1_pul_cnt_addr_lst[3 ];
    lvds_grp_pul_cnt_addr_lst[13] =   gpi_grp1_pul_cnt_addr_lst[2 ];
    lvds_grp_pul_cnt_addr_lst[14] =   gpi_grp1_pul_cnt_addr_lst[1 ];
    lvds_grp_pul_cnt_addr_lst[15] =   gpi_grp1_pul_cnt_addr_lst[0 ];
    lvds_grp_pul_cnt_addr_lst[16] =   gpi_grp2_pul_cnt_addr_lst[15];
    lvds_grp_pul_cnt_addr_lst[17] =   gpi_grp2_pul_cnt_addr_lst[14];
    lvds_grp_pul_cnt_addr_lst[18] =   gpi_grp2_pul_cnt_addr_lst[13];
    lvds_grp_pul_cnt_addr_lst[19] =   gpi_grp2_pul_cnt_addr_lst[12];
    lvds_grp_pul_cnt_addr_lst[20] =   gpi_grp2_pul_cnt_addr_lst[11];
    lvds_grp_pul_cnt_addr_lst[21] =   gpi_grp2_pul_cnt_addr_lst[10];
    lvds_grp_pul_cnt_addr_lst[22] =   gpi_grp2_pul_cnt_addr_lst[9 ];
    lvds_grp_pul_cnt_addr_lst[23] =   gpi_grp2_pul_cnt_addr_lst[8 ];
    lvds_grp_pul_cnt_addr_lst[24] =   gpi_grp2_pul_cnt_addr_lst[7 ];
    lvds_grp_pul_cnt_addr_lst[25] =   gpi_grp2_pul_cnt_addr_lst[6 ];
    lvds_grp_pul_cnt_addr_lst[26] =   gpi_grp2_pul_cnt_addr_lst[5 ];
    lvds_grp_pul_cnt_addr_lst[27] =   gpi_grp2_pul_cnt_addr_lst[4 ];
    lvds_grp_pul_cnt_addr_lst[28] =   gpi_grp2_pul_cnt_addr_lst[3 ];
    lvds_grp_pul_cnt_addr_lst[29] =   gpi_grp2_pul_cnt_addr_lst[2 ];
    lvds_grp_pul_cnt_addr_lst[30] =   gpi_grp2_pul_cnt_addr_lst[1 ];
    lvds_grp_pul_cnt_addr_lst[31] =   gpi_grp2_pul_cnt_addr_lst[0 ];
    lvds_grp_pul_cnt_addr_lst[32] =   gpi_grp3_pul_cnt_addr_lst[15];
    lvds_grp_pul_cnt_addr_lst[33] =   gpi_grp3_pul_cnt_addr_lst[14];
    lvds_grp_pul_cnt_addr_lst[34] =   gpi_grp3_pul_cnt_addr_lst[13];
    lvds_grp_pul_cnt_addr_lst[35] =   gpi_grp3_pul_cnt_addr_lst[12];
    lvds_grp_pul_cnt_addr_lst[36] =   gpi_grp3_pul_cnt_addr_lst[11];
    lvds_grp_pul_cnt_addr_lst[37] =   gpi_grp3_pul_cnt_addr_lst[10];
    lvds_grp_pul_cnt_addr_lst[38] =   gpi_grp3_pul_cnt_addr_lst[9 ];
    lvds_grp_pul_cnt_addr_lst[39] =   gpi_grp3_pul_cnt_addr_lst[8 ];
    lvds_grp_pul_cnt_addr_lst[40] =   gpi_grp3_pul_cnt_addr_lst[7 ];
    lvds_grp_pul_cnt_addr_lst[41] =   gpi_grp3_pul_cnt_addr_lst[6 ];
    lvds_grp_pul_cnt_addr_lst[42] =   gpi_grp3_pul_cnt_addr_lst[5 ];
    lvds_grp_pul_cnt_addr_lst[43] =   gpi_grp3_pul_cnt_addr_lst[4 ];
    lvds_grp_pul_cnt_addr_lst[44] =   gpi_grp3_pul_cnt_addr_lst[3 ];
    lvds_grp_pul_cnt_addr_lst[45] =   gpi_grp3_pul_cnt_addr_lst[2 ];
    lvds_grp_pul_cnt_addr_lst[46] =   gpi_grp3_pul_cnt_addr_lst[1 ];
    lvds_grp_pul_cnt_addr_lst[47] =   gpi_grp3_pul_cnt_addr_lst[0 ];
    lvds_grp_pul_cnt_addr_lst[48] =   gpi_grp4_pul_cnt_addr_lst[15];
    lvds_grp_pul_cnt_addr_lst[49] =   gpi_grp4_pul_cnt_addr_lst[14];
    lvds_grp_pul_cnt_addr_lst[50] =   gpi_grp4_pul_cnt_addr_lst[13];
    lvds_grp_pul_cnt_addr_lst[51] =   gpi_grp4_pul_cnt_addr_lst[12];
    lvds_grp_pul_cnt_addr_lst[52] =   gpi_grp4_pul_cnt_addr_lst[11];
    lvds_grp_pul_cnt_addr_lst[53] =   gpi_grp4_pul_cnt_addr_lst[10];
    lvds_grp_pul_cnt_addr_lst[54] =   gpi_grp4_pul_cnt_addr_lst[9 ];
    lvds_grp_pul_cnt_addr_lst[55] =   gpi_grp5_pul_cnt_addr_lst[3 ];

    rs422_grp_pul_cnt_addr_lst.assign(4,0x00);
    rs422_grp_pul_cnt_addr_lst[0] = gpi_grp4_pul_cnt_addr_lst[8];
    rs422_grp_pul_cnt_addr_lst[1] = gpi_grp4_pul_cnt_addr_lst[7];
    rs422_grp_pul_cnt_addr_lst[2] = gpi_grp4_pul_cnt_addr_lst[6];
    rs422_grp_pul_cnt_addr_lst[3] = gpi_grp4_pul_cnt_addr_lst[5];

    lvttl_grp_pul_cnt_addr_lst.assign(8,0x00);
    lvttl_grp_pul_cnt_addr_lst[0] = gpi_grp4_pul_cnt_addr_lst[4];
    lvttl_grp_pul_cnt_addr_lst[1] = gpi_grp4_pul_cnt_addr_lst[3];
    lvttl_grp_pul_cnt_addr_lst[2] = gpi_grp4_pul_cnt_addr_lst[2];
    lvttl_grp_pul_cnt_addr_lst[3] = gpi_grp4_pul_cnt_addr_lst[1];
    lvttl_grp_pul_cnt_addr_lst[4] = gpi_grp4_pul_cnt_addr_lst[0];
    lvttl_grp_pul_cnt_addr_lst[5] = gpi_grp5_pul_cnt_addr_lst[2];
    lvttl_grp_pul_cnt_addr_lst[6] = gpi_grp5_pul_cnt_addr_lst[1];
    lvttl_grp_pul_cnt_addr_lst[7] = gpi_grp5_pul_cnt_addr_lst[0];

}

auto_mode::~auto_mode()
{
    delete ui;
}

void auto_mode::on_lvds_tx_en_clicked(bool checked)
{
    for (QAbstractButton *button : lvds_cn_rx2_tx_group->buttons()) {
        button->setChecked(checked);
    }
    for (QAbstractButton *button : lvds_cn_rx2_tx_dis_group->buttons()) {
        button->setChecked(checked);
    }

}

void auto_mode::on_cn_tx_cnfg_clicked()
{
    ui->cn_tx_cnfg->setEnabled(false);
// GPO Group 1
    if(ui->lvds_tx_en_1->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x1;
    }
    else {
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0xFFFE;
    }

    if(ui->lvds_tx_en_2->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x2;
    }
    else{
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0xFFFD;
    }

    if(ui->lvds_tx_en_3->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x4;
    }
    else{
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0xFFFB;
    }

    if(ui->lvds_tx_en_4->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x8;
    }
    else{
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0xFFF7;
    }

    if(ui->lvds_tx_en_5->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x10;
    }
    else{
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0xFFEF;
    }

    if(ui->lvds_tx_en_6->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x20;
    }
    else{
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0xFFDF;
    }

    if(ui->lvds_tx_en_7->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x40;
    }
    else{
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0xFFBF;
    }

    if(ui->lvds_tx_en_8->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x80;
    }
    else{
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0xFF7F;
    }

    if(ui->lvds_tx_en_9->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x100;
    }
    else{
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0xFEFF;
    }

    if(ui->lvds_tx_en_10->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x200;
    }
    else{
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0xFDFF;
    }

    if(ui->lvds_tx_en_11->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x400;
    }
    else{
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0xFBFF;
    }

    if(ui->lvds_tx_en_12->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x800;
    }
    else{
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0xF7FF;
    }

    if(ui->lvds_tx_en_13->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x1000;
    }
    else{
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0xEFFF;
    }

    if(ui->lvds_tx_en_14->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x2000;
    }
    else{
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0xDFFF;
    }

    if(ui->lvds_tx_en_15->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x4000;
    }
    else{
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0xBFFF;
    }

    if(ui->lvds_tx_en_16->isChecked()){
        gpo_grp1_en_sts = gpo_grp1_en_sts | 0x8000;
    }
    else{
        gpo_grp1_en_sts = gpo_grp1_en_sts & 0x7FFF;
    }
    uart_write(gpo_grp1_addr,gpo_grp1_en_sts,true);
    serial->waitForBytesWritten(10);
    serial->waitForReadyRead(50);
    status = serial->readAll();
    if(static_cast<quint8>(status[0]) == 0x50){
        ui->action_log->setTextColor(Qt::green);
        ui->action_log->setText("TX Group 1 configuration success");
    }
    else{
        ui->action_log->setTextColor(Qt::red);
        ui->action_log->setText("TX Group 1 configuration failed");
    }



    // GPO Group 2
    if(ui->lvds_tx_en_17->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x1;
    }
    else {
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0xFFFE;
    }

    if(ui->lvds_tx_en_18->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x2;
    }
    else{
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0xFFFD;
    }

    if(ui->lvds_tx_en_19->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x4;
    }
    else{
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0xFFFB;
    }

    if(ui->lvds_tx_en_20->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x8;
    }
    else{
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0xFFF7;
    }

    if(ui->lvds_tx_en_21->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x10;
    }
    else{
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0xFFEF;
    }

    if(ui->lvds_tx_en_22->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x20;
    }
    else{
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0xFFDF;
    }

    if(ui->lvds_tx_en_23->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x40;
    }
    else{
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0xFFBF;
    }

    if(ui->lvds_tx_en_24->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x80;
    }
    else{
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0xFF7F;
    }

    if(ui->lvds_tx_en_25->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x100;
    }
    else{
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0xFEFF;
    }

    if(ui->lvds_tx_en_26->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x200;
    }
    else{
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0xFDFF;
    }

    if(ui->lvds_tx_en_27->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x400;
    }
    else{
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0xFBFF;
    }

    if(ui->lvds_tx_en_28->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x800;
    }
    else{
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0xF7FF;
    }

    if(ui->lvds_tx_en_29->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x1000;
    }
    else{
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0xEFFF;
    }

    if(ui->lvds_tx_en_30->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x2000;
    }
    else{
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0xDFFF;
    }

    if(ui->lvds_tx_en_31->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x4000;
    }
    else{
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0xBFFF;
    }

    if(ui->lvds_tx_en_32->isChecked()){
        gpo_grp2_en_sts = gpo_grp2_en_sts | 0x8000;
    }
    else{
        gpo_grp2_en_sts = gpo_grp2_en_sts & 0x7FFF;
    }
    uart_write(gpo_grp2_addr,gpo_grp2_en_sts,true);
    serial->waitForBytesWritten(10);
    serial->waitForReadyRead(50);
    status = serial->readAll();
    if(static_cast<quint8>(status[0]) == 0x50){
        ui->action_log->setTextColor(Qt::green);
        ui->action_log->setText("TX Group 2 configuration success");
    }
    else{
        ui->action_log->setTextColor(Qt::red);
        ui->action_log->setText("TX Group 2 configuration failed");
    }

    // GPO Group 3
    if(ui->lvds_tx_en_33->isChecked()){
        gpo_grp3_en_sts = gpo_grp3_en_sts | 0x1;
    }
    else {
        gpo_grp3_en_sts = gpo_grp3_en_sts & 0xFFFE;
    }
    uart_write(gpo_grp3_addr,gpo_grp3_en_sts,true);
    serial->waitForBytesWritten(10);
    serial->waitForReadyRead(50);
    status = serial->readAll();
    if(static_cast<quint8>(status[0]) == 0x50){
        ui->action_log->setTextColor(Qt::green);
        ui->action_log->setText("TX Group 3 configuration success");
    }
    else{
        ui->action_log->setTextColor(Qt::red);
        ui->action_log->setText("TX Group 3 configuration failed");
    }
    ui->cn_tx_cnfg->setEnabled(true);
}

void auto_mode::on_lvttl_tx_en_clicked(bool checked)
{

    ui->lvttl_tx_en_35    ->setChecked(checked);
    ui->lvttl_tx_en_36    ->setChecked(checked);
    ui->lvttl_tx_en_37    ->setChecked(checked);
    ui->lvttl_tx_en_38    ->setChecked(checked);
    ui->lvttl_tx_en_39    ->setChecked(checked);
    ui->lvttl_tx_en_40    ->setChecked(checked);
    ui->lvttl_tx_en_41    ->setChecked(checked);
}

void auto_mode::on_rs422_tx_en_clicked(bool checked)
{
    ui->rs422_tx_en_34    ->setChecked(checked);
}

void auto_mode::on_tx_clicked(bool checked)
{
    ui->lvds_tx_en->setChecked(checked);
    ui->lvds_tx_en_1    ->setChecked(checked);
    ui->lvds_tx_en_2    ->setChecked(checked);
    ui->lvds_tx_en_3    ->setChecked(checked);
    ui->lvds_tx_en_4    ->setChecked(checked);
    ui->lvds_tx_en_5    ->setChecked(checked);
    ui->lvds_tx_en_6    ->setChecked(checked);
    ui->lvds_tx_en_7    ->setChecked(checked);
    ui->lvds_tx_en_8    ->setChecked(checked);
    ui->lvds_tx_en_9    ->setChecked(checked);
    ui->lvds_tx_en_10   ->setChecked(checked);
    ui->lvds_tx_en_11   ->setChecked(checked);
    ui->lvds_tx_en_12   ->setChecked(checked);
    ui->lvds_tx_en_13   ->setChecked(checked);
    ui->lvds_tx_en_14   ->setChecked(checked);
    ui->lvds_tx_en_15   ->setChecked(checked);
    ui->lvds_tx_en_16   ->setChecked(checked);
    ui->lvds_tx_en_17   ->setChecked(checked);
    ui->lvds_tx_en_18   ->setChecked(checked);
    ui->lvds_tx_en_19   ->setChecked(checked);
    ui->lvds_tx_en_20   ->setChecked(checked);
    ui->lvds_tx_en_21   ->setChecked(checked);
    ui->lvds_tx_en_22   ->setChecked(checked);
    ui->lvds_tx_en_23   ->setChecked(checked);
    ui->lvds_tx_en_24   ->setChecked(checked);
    ui->lvds_tx_en_25   ->setChecked(checked);
    ui->lvds_tx_en_26   ->setChecked(checked);
    ui->lvds_tx_en_27   ->setChecked(checked);
    ui->lvds_tx_en_28   ->setChecked(checked);
    ui->lvds_tx_en_29   ->setChecked(checked);
    ui->lvds_tx_en_30   ->setChecked(checked);
    ui->lvds_tx_en_31   ->setChecked(checked);
    ui->lvds_tx_en_32   ->setChecked(checked);
    ui->lvds_tx_en_33   ->setChecked(checked);

    ui->rs422_tx_en->setChecked(checked);
    ui->rs422_tx_en_34    ->setChecked(checked);

    ui->lvttl_tx_en->setChecked(checked);
    ui->lvttl_tx_en_35    ->setChecked(checked);
    ui->lvttl_tx_en_36    ->setChecked(checked);
    ui->lvttl_tx_en_37    ->setChecked(checked);
    ui->lvttl_tx_en_38    ->setChecked(checked);
    ui->lvttl_tx_en_39    ->setChecked(checked);
    ui->lvttl_tx_en_40    ->setChecked(checked);
    ui->lvttl_tx_en_41    ->setChecked(checked);

}

void auto_mode::on_rs422_tx_cnfg_clicked()
{
    ui->rs422_tx_cnfg->setEnabled(false);
    // GPO Group 3
    if(ui->rs422_tx_en_34->isChecked()){
        gpo_grp3_en_sts = gpo_grp3_en_sts | 0x2;
    }
    else {
        gpo_grp3_en_sts = gpo_grp3_en_sts & 0xFFFD;
    }
    uart_write(gpo_grp3_addr,gpo_grp3_en_sts,true);
    serial->waitForBytesWritten(10);
    serial->waitForReadyRead(50);
    status = serial->readAll();
    if(static_cast<quint8>(status[0]) == 0x50){
        ui->action_log->setTextColor(Qt::green);
        ui->action_log->setText("TX Group 3 configuration success");
    }
    else{
        ui->action_log->setTextColor(Qt::red);
        ui->action_log->setText("TX Group 3 configuration failed");
    }
    ui->rs422_tx_cnfg->setEnabled(true);
}

void auto_mode::on_lvttl_tx_cnfg_clicked()
{
    ui->lvttl_tx_cnfg->setEnabled(false);
    // GPO Group 3
    if(ui->lvttl_tx_en_35->isChecked()){
        gpo_grp3_en_sts = gpo_grp3_en_sts | 0x4;
    }
    else {
        gpo_grp3_en_sts = gpo_grp3_en_sts & 0xFFFB;
    }

    if(ui->lvttl_tx_en_36->isChecked()){
        gpo_grp3_en_sts = gpo_grp3_en_sts | 0x8;
    }
    else {
        gpo_grp3_en_sts = gpo_grp3_en_sts & 0xFFF7;
    }

    if(ui->lvttl_tx_en_37->isChecked()){
        gpo_grp3_en_sts = gpo_grp3_en_sts | 0x10;
    }
    else {
        gpo_grp3_en_sts = gpo_grp3_en_sts & 0xFFEF;
    }

    if(ui->lvttl_tx_en_38->isChecked()){
        gpo_grp3_en_sts = gpo_grp3_en_sts | 0x20;
    }
    else {
        gpo_grp3_en_sts = gpo_grp3_en_sts & 0xFFDF;
    }

    if(ui->lvttl_tx_en_39->isChecked()){
        gpo_grp3_en_sts = gpo_grp3_en_sts | 0x40;
    }
    else {
        gpo_grp3_en_sts = gpo_grp3_en_sts & 0xFFBF;
    }

    if(ui->lvttl_tx_en_40->isChecked()){
        gpo_grp3_en_sts = gpo_grp3_en_sts | 0x80;
    }
    else {
        gpo_grp3_en_sts = gpo_grp3_en_sts & 0xFF7F;
    }

    if(ui->lvttl_tx_en_41->isChecked()){
        gpo_grp3_en_sts = gpo_grp3_en_sts | 0x100;
    }
    else {
        gpo_grp3_en_sts = gpo_grp3_en_sts & 0xFEFF;
    }
    uart_write(gpo_grp3_addr,gpo_grp3_en_sts,true);
    serial->waitForBytesWritten(10);
    serial->waitForReadyRead(50);
    status = serial->readAll();
    if(static_cast<quint8>(status[0]) == 0x50){
        ui->action_log->setTextColor(Qt::green);
        ui->action_log->setText("TX Group 3 configuration success");
    }
    else{
        ui->action_log->setTextColor(Qt::red);
        ui->action_log->setText("TX Group 3 configuration failed");
    }
    ui->lvttl_tx_cnfg->setEnabled(true);
}

void auto_mode::on_lvds_rx_en_clicked(bool checked)
{
    for (QAbstractButton *button : lvds_cn_rx1_group->buttons()) {
            button->setChecked(checked);
        }
    for (QAbstractButton *button : lvds_cn_rx2_group->buttons()) {
            button->setChecked(checked);
        }
}

void auto_mode::on_rs422_rx_en_clicked(bool checked)
{
    for (QAbstractButton *button : rs422_rx_group->buttons()) {
            button->setChecked(checked);
        }
}

void auto_mode::on_lvttl_rx_en_clicked(bool checked)
{
    for (QAbstractButton *button : lvttl_rx_group->buttons()) {
            button->setChecked(checked);
        }
}

void auto_mode::on_rx_clicked(bool checked)
{
    ui->lvds_rx_en->setChecked(checked);
    ui->lvttl_rx_en->setChecked(checked);
    ui->rs422_rx_en->setChecked(checked);
    ui->rs422_rx_en->setChecked(checked);
    ui->cn_rx1->setChecked(false);
    ui->cn_rx2->setChecked(false);
    ui->rs422_rx1->setChecked(false);
    ui->rs422_rx2->setChecked(false);
    for (QAbstractButton *button : lvds_cn_rx1_group->buttons()) {
            button->setChecked(checked);
        }
    for (QAbstractButton *button : lvds_cn_rx2_group->buttons()) {
            button->setChecked(checked);
        }
    for (QAbstractButton *button : rs422_rx_group->buttons()) {
            button->setChecked(checked);
        }
    for (QAbstractButton *button : lvttl_rx_group->buttons()) {
            button->setChecked(checked);
        }
}

void auto_mode::on_ref_clicked()
{
    ui->com_port->clear();
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &portInfo : serialPortInfos) {
            ui->com_port->addItem(portInfo.portName());
        }
}

void auto_mode::on_connect_clicked()
{
    ui->connect->setEnabled(false);
    if(ui->connect->text() == "CONNECT"){
        // 1. Set the Port Name from the Combo Box
        QString portName = ui->com_port->currentText();
        if (portName.isEmpty()) {
            QMessageBox::warning(this, "Selection Error", "Please select a valid COM port.");
            return;
        }
        serial->setPortName(portName);

        // 2. Set UART Configurations (Hardcoded or from UI)
        if (!serial->setBaudRate(921600)) {
            QMessageBox::warning(this, "Baud Rate Error", "Your hardware may not support 921600 baud.");
        }
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);

        // 3. Attempt to Open the Port
        if (serial->open(QIODevice::ReadWrite)) {
            ui->connect->setText("DISCONNECT");
            ui->ref->setEnabled(false);
            ui->com_port->setEnabled(false);
            ui->action_log->setTextColor(Qt::green);
            ui->action_log->setText("Com port open success");
        } else {
            ui->action_log->setTextColor(Qt::red);
            ui->action_log->setText("Failed to open COM Port");
        }

    }
    else{
        // If the port is already open, the button should "Disconnect"
        if (serial->isOpen()) {
            serial->close();
            ui->connect->setText("CONNECT");
            ui->ref->setEnabled(true);
            ui->com_port->setEnabled(true);
        }
    }
    ui->connect->setEnabled(true);
}

void auto_mode::on_read_clicked()
{
    ui->read->setEnabled(false);
    if (!serial->isOpen()) {
        QMessageBox::warning(this, "Port Closed", "Please connect to a COM port first!");
        return;
    }


    // 3. Convert Hex String to QByteArray
    QByteArray frame;

    // Byte 0: Header
    frame.append(static_cast<char>(m_header1));

    // Byte 1 & 2: [R/W bit (1 bit)] + [Address (15 bits)]
    // Pack R/W into the most significant bit of the 16-bit block
    QString addrStr = ui->address_edt->text().trimmed();

        // 2. Convert Hex String to Unsigned Int (Base 16)
        bool ok;
        uint32_t tempAddr = addrStr.toUInt(&ok, 16);

        // 3. Store in the variable if conversion was successful
        if (ok) {
            // Mask to 15 bits as per your frame requirement
            m_address1 = static_cast<uint16_t>(tempAddr & 0x7FFF);
        } else {
            QMessageBox::warning(this, "Input Error", "Invalid Address! Please enter a Hex value (e.g., 1A2B)");
            return; // Stop the function if address is bad
        }
    uint16_t addrBlock = (m_isWrite1 ? 0x0000 : 0x0000) | m_address1;
    frame.append(static_cast<char>((addrBlock >> 8) & 0xFF)); // High Byte
    frame.append(static_cast<char>(addrBlock & 0xFF));        // Low Byte

    // Byte 3 & 4: Data (16 bit)
    //frame.append(static_cast<char>((m_data1 >> 8) & 0xFF));    // MSB
    //frame.append(static_cast<char>(m_data1 & 0xFF));           // LSB

    // Byte 5: XOR Checksum (Calculated over all 5 previous bytes)
    m_checksum1 = 0;
    for(int i = 0; i < frame.size(); ++i) {
        m_checksum1 ^= static_cast<uint8_t>(frame.at(i));
    }
    frame.append(static_cast<char>(m_checksum1));

    // 4. Send over Serial
    qint64 bytesWritten = serial->write(frame);
    serial->waitForBytesWritten(10);


    if (bytesWritten == -1) {
        QMessageBox::critical(this, "Error", "Failed to write data: " + serial->errorString());
        frame.clear();
    } else {

        if (serial->waitForReadyRead(50)) {
                QByteArray data = serial->readAll();
//                data = qToBigEndian(data);
                data = data.mid(1,2);
                quint16 dataa = 0;
                dataa = (static_cast<quint8>(data[1]) << 8) | (static_cast<quint8>(data[2]));
                ui->data_edt->setText(data.toHex(' ').toUpper());
                frame.clear();
                ui->action_log->setTextColor(Qt::green);
                ui->action_log->setText("Debuf read success");
            } else {
                ui->action_log->setTextColor(Qt::red);
                ui->action_log->setText("Debuf read failed");

            }
        serial->flush();
    }
    ui->read->setEnabled(true);
}

void auto_mode::on_write_clicked()
{
    ui->write->setEnabled(false);
    if (!serial->isOpen()) {
        QMessageBox::warning(this, "Port Closed", "Please connect to a COM port first!");
        return;
    }

    // 3. Convert Hex String to QByteArray
    QByteArray frame;

    // Byte 0: Header
    frame.append(static_cast<char>(m_header1));

    // Byte 1 & 2: [R/W bit (1 bit)] + [Address (15 bits)]
    // Pack R/W into the most significant bit of the 16-bit block
    QString addrStr = ui->address_edt->text().trimmed();

        // 2. Convert Hex String to Unsigned Int (Base 16)
        bool ok;
        uint32_t tempAddr = addrStr.toUInt(&ok, 16);

        // 3. Store in the variable if conversion was successful
        if (ok) {
            // Mask to 15 bits as per your frame requirement
            m_address1 = static_cast<uint16_t>(tempAddr & 0x7FFF);
        } else {
            QMessageBox::warning(this, "Input Error", "Invalid Address! Please enter a Hex value (e.g., 1A2B)");
            return; // Stop the function if address is bad
        }

    QString dataStr = ui->data_edt->text().trimmed();

            // 2. Convert Hex String to Unsigned Int (Base 16)

    m_data1 = dataStr.toUInt(&ok, 16);

    uint16_t addrBlock = (m_isWrite1 ? 0x8000 : 0x0000) | m_address1;
    frame.append(static_cast<char>((addrBlock >> 8) & 0xFF)); // High Byte
    frame.append(static_cast<char>(addrBlock & 0xFF));        // Low Byte

    // Byte 3 & 4: Data (16 bit)
    frame.append(static_cast<char>((m_data1 >> 8) & 0xFF));    // MSB
    frame.append(static_cast<char>(m_data1 & 0xFF));           // LSB

    // Byte 5: XOR Checksum (Calculated over all 5 previous bytes)
    m_checksum1 = 0;
    for(int i = 0; i < frame.size(); ++i) {
        m_checksum1 ^= static_cast<uint8_t>(frame.at(i));
    }
    frame.append(static_cast<char>(m_checksum1));

    // 4. Send over Serial
    qint64 bytesWritten = serial->write(frame);
    serial->waitForBytesWritten(10);
    serial->waitForReadyRead(50);
    status = serial->readAll();
    if(static_cast<quint8>(status[0]) == 0x50){
        ui->action_log->setTextColor(Qt::green);
        ui->action_log->setText("Debug write success");
    }
    else{
        ui->action_log->setTextColor(Qt::red);
        ui->action_log->setText("Debug write failed");
    }
    ui->write->setEnabled(true);
}

void auto_mode::uart_write(uint16_t address, uint16_t data, bool isWrite)
{
    if (!serial->isOpen()) return;

    QByteArray frame;

    // Byte 0: Header
    frame.append(static_cast<char>(m_header1));

    // Byte 1 & 2: R/W bit + 15-bit Address
    uint16_t addrBlock = (isWrite ? 0x8000 : 0x0000) | (address & 0x7FFF);
    frame.append(static_cast<char>((addrBlock >> 8) & 0xFF)); // High Byte
    frame.append(static_cast<char>(addrBlock & 0xFF));        // Low Byte

    // Byte 3 & 4: Data (16 bit)
    frame.append(static_cast<char>((data >> 8) & 0xFF));    // MSB
    frame.append(static_cast<char>(data & 0xFF));           // LSB

    // Byte 5: XOR Checksum
    uint8_t checksum = 0;
    for(int i = 0; i < frame.size(); ++i) {
        checksum ^= static_cast<uint8_t>(frame.at(i));
    }
    frame.append(static_cast<char>(checksum));

    // Send
    serial->write(frame);
    serial->flush();
}

quint16 auto_mode::uart_read(uint16_t address)
{
    uint16_t data = 0;
    if (!serial->isOpen()) {
        QMessageBox::warning(this, "Port Closed", "Please connect to a COM port first!");
    }
    else
    {
        // 3. Convert Hex String to QByteArray
        QByteArray frame;
        // Byte 0: Header
        m_address1 = address;
        frame.append(static_cast<char>(m_header1));
        uint16_t addrBlock = 0x7FFF & m_address1;
        frame.append(static_cast<char>((addrBlock >> 8) & 0xFF)); // High Byte
        frame.append(static_cast<char>(addrBlock & 0xFF));        // Low Byte

        // Byte 3 & 4: Data (16 bit)
        //frame.append(static_cast<char>((m_data1 >> 8) & 0xFF));    // MSB
        //frame.append(static_cast<char>(m_data1 & 0xFF));           // LSB

        // Byte 5: XOR Checksum (Calculated over all 5 previous bytes)
        m_checksum1 = 0;
        for(int i = 0; i < frame.size(); ++i) {
            m_checksum1 ^= static_cast<uint8_t>(frame.at(i));
        }
        frame.append(static_cast<char>(m_checksum1));

        // 4. Send over Serial
        qint64 bytesWritten = serial->write(frame);
        serial->waitForBytesWritten(10);

        if (bytesWritten == -1) {
            QMessageBox::critical(this, "Error", "Failed to write data: " + serial->errorString());
            frame.clear();
        } else {

            if (serial->waitForReadyRead(60)) {
                    QByteArray dataa = serial->readAll();
                    data = (static_cast<quint8>(dataa[2]) << 8) | static_cast<quint8>(dataa[1]);
                    serial->flush();
                }
            else{
                data = 0xB055;

            }
        }
    }
    return data;
}

void auto_mode::on_freq_cnfg_clicked()
{
    QString freqStr = ui->freq_ip->text().trimmed();

        // 2. Convert Hex String to Unsigned Int (Base 16)
        bool ok;
        req_freq = freqStr.toUInt(&ok);
        pul_prd_time = (1.0/req_freq)/(1.0/smp_freq);
        valid_pul_time = (1.0/req_freq)/(1.0/smp_freq);
        on_time = ((1.0/req_freq)/(1.0/smp_freq))/2.0;
        uart_write(on_pul_reg_addr,on_time,true);
        serial->waitForBytesWritten(10);
        serial->waitForReadyRead(50);
        status = serial->readAll();
        if(static_cast<quint8>(status[0]) == 0x50){
            ui->action_log->setTextColor(Qt::green);
            ui->action_log->setText("ON Pulse configiration success");
        }
        else{
            ui->action_log->setTextColor(Qt::red);
            ui->action_log->setText("ON Pulse configiration failed");
        }
        uart_write(period_reg_lsb_addr,pul_prd_time,true);
        serial->waitForBytesWritten(10);
        serial->waitForReadyRead(50);
        status = serial->readAll();
        if(static_cast<quint8>(status[0]) == 0x50){
            ui->action_log->setTextColor(Qt::green);
            ui->action_log->setText("Pulse period configiration success");
        }
        else{
            ui->action_log->setTextColor(Qt::red);
            ui->action_log->setText("Pulse period configiration failed");
        }
        uart_write(pul_valid_addr,pul_prd_time,true);
        serial->waitForBytesWritten(10);
        serial->waitForReadyRead(50);
        status = serial->readAll();
        if(static_cast<quint8>(status[0]) == 0x50){
            ui->action_log->setTextColor(Qt::green);
            ui->action_log->setText("valid Pulse period configiration success");
        }
        else{
            ui->action_log->setTextColor(Qt::red);
            ui->action_log->setText("valid Pulse period configiration failed");
        }
}

void auto_mode::on_tx_enable_clicked()
{
    ui->tx_enable->setEnabled(false);
    if(ui->tx_enable->text()== "ENABLE"){
        uart_write(pul_enable_addr,0X0001,true);
        serial->waitForBytesWritten(10);
        serial->waitForReadyRead(50);
        status = serial->readAll();
        if(static_cast<quint8>(status[0]) == 0x50){
            ui->action_log->setTextColor(Qt::green);
            ui->action_log->setText("TX enable configiration success");
        }
        else{
            ui->action_log->setTextColor(Qt::red);
            ui->action_log->setText("TX enable configiration failed");
        }
        ui->tx_enable->setText("DISABLE");
    }
    else{
        uart_write(pul_enable_addr,0X0000,true);
        serial->waitForBytesWritten(10);
        serial->waitForReadyRead(50);
        status = serial->readAll();
        if(static_cast<quint8>(status[0]) == 0x50){
            ui->action_log->setTextColor(Qt::green);
            ui->action_log->setText("TX diable configiration success");
        }
        else{
            ui->action_log->setTextColor(Qt::red);
            ui->action_log->setText("TX dsiable configiration failed");
        }
        ui->tx_enable->setText("ENABLE");
    }
    ui->tx_enable->setEnabled(true);
}


void auto_mode::on_lvds_rx_read_clicked()
{
    ui->lvds_rx_read->setEnabled(false);
    quint16 freq = 0;
    quint16 glitch_cnt = 0;
    int counter = 0;
    // Iterate through every button in the group
    // The loop now knows what 'lvds_rx_group' is
    for (QAbstractButton *button : rx_lvds_group[0]->buttons()) {
        if (button->isChecked()) {
            freq = uart_read(lvds_grp_pul_cnt_addr_lst[lvds_rx_index[counter]]);
            glitch_cnt = uart_read(lvds_grp_glitch_cnt_addr_lst[lvds_rx_index[counter]]);
            lvds_freq_group[counter]->setDigitCount(4);
            lvds_glitch_group[counter]->setDigitCount(4);
            if(freq == 0x03 | freq == 0x01){
                lvds_freq_group[counter]->display("PASS");
                lvds_glitch_group[counter]->display(glitch_cnt);
            }
            else if(freq == 0x00)
            {
                lvds_freq_group[counter]->display("FA1L");
                lvds_glitch_group[counter]->display(glitch_cnt);
            }
            else if(freq != 0xB055){
                ui->action_log->setTextColor(Qt::red);
                ui->action_log->setText("LVDS RX UART Read Failed");
            }
        }
        else{
            lvds_freq_group[counter]->setDigitCount(5);
            lvds_glitch_group[counter]->setDigitCount(5);
            lvds_freq_group[counter]->display("-----");
            lvds_glitch_group[counter]->display("-----");

        }
        counter++;
    }

    for (QAbstractButton *button : rx_lvds_group[1]->buttons()) {
        if (button->isChecked()) {
            freq = uart_read(lvds_grp_pul_cnt_addr_lst[lvds_rx_index[counter]]);
            glitch_cnt = uart_read(lvds_grp_glitch_cnt_addr_lst[counter]);
            lvds_freq_group[counter]->setDigitCount(4);
            lvds_glitch_group[counter]->setDigitCount(4);
            if(freq == 0x03 | freq == 0x01){
                lvds_freq_group[counter]->display("PASS");
                lvds_glitch_group[counter]->display(glitch_cnt);
            }
            else if(freq == 0x03 | freq == 0x00)
            {
                lvds_freq_group[counter]->display("FA1L");
                lvds_glitch_group[counter]->display(glitch_cnt);
            }
            else if(freq != 0xB055){
                ui->action_log->setTextColor(Qt::red);
                ui->action_log->setText("LVDS RX UART Read Failed");
            }
        }
        else{
            lvds_freq_group[counter]->setDigitCount(5);
            lvds_glitch_group[counter]->setDigitCount(5);
            lvds_freq_group[counter]->display("-----");
            lvds_glitch_group[counter]->display("-----");

        }
        counter++;
    }
    ui->lvds_rx_read->setEnabled(true);
}


void auto_mode::on_tx_config_clicked()
{
    auto_mode::on_cn_tx_cnfg_clicked();
    auto_mode::on_rs422_tx_cnfg_clicked();
    auto_mode::on_lvttl_tx_cnfg_clicked();
}


void auto_mode::on_lvttl_rx_read_clicked()
{
    ui->lvttl_rx_read->setEnabled(false);
    quint16 freq = 0;
    quint16 glitch_cnt = 0;
    int counter = 0;
    // Iterate through every button in the group
    // The loop now knows what 'lvds_rx_group' is
    for (QAbstractButton *button : lvttl_rx_group->buttons()) {
        if (button->isChecked()) {
            freq = uart_read(lvttl_grp_pul_cnt_addr_lst[counter]);
            glitch_cnt = uart_read(lvttl_grp_glitch_cnt_addr_lst[counter]);
            lvttl_freq_group[counter]->setDigitCount(5);
            lvttl_glitch_group[counter]->setDigitCount(5);
            if(freq == 0x03 | freq == 0x01){
                lvttl_freq_group[counter]->display("PASS");
                lvttl_glitch_group[counter]->display(glitch_cnt);
            }
            else if(freq == 0x00)
            {
                lvttl_freq_group[counter]->display("FA1L");
                lvttl_glitch_group[counter]->display(glitch_cnt);
            }
            else if(freq != 0xB055){
                ui->action_log->setTextColor(Qt::red);
                ui->action_log->setText("LVDS RX UART Read Failed");
            }
        }
        else{
            lvttl_freq_group[counter]->setDigitCount(5);
            lvttl_glitch_group[counter]->setDigitCount(5);
            lvttl_freq_group[counter]->display("-----");
            lvttl_glitch_group[counter]->display("-----");

        }
        counter++;
    }
    ui->lvttl_rx_read->setEnabled(true);
}


void auto_mode::on_RS422_rx_read_clicked()
{
    ui->RS422_rx_read->setEnabled(false);
    quint16 freq = 0;
    quint16 glitch_cnt = 0;
    int counter = 0;
    // Iterate through every button in the group
    // The loop now knows what 'lvds_rx_group' is
    for (QAbstractButton *button : rs422_rx_group->buttons()) {
        if (button->isChecked()) {
            freq = uart_read(rs422_grp_pul_cnt_addr_lst[counter]);
            glitch_cnt = uart_read(rs422_grp_glitch_cnt_addr_lst[counter]);
            rs422_freq_group[counter]->setDigitCount(5);
            rs422_glitch_group[counter]->setDigitCount(5);
            if(freq == 0x01){
                rs422_freq_group[counter]->display("PASS");
                rs422_glitch_group[counter]->display(glitch_cnt);
            }
            else if(freq == 0x03 | freq == 0x000)
            {
                rs422_freq_group[counter]->display("FA1L");
                rs422_glitch_group[counter]->display(glitch_cnt);
            }
            else if(freq != 0xB055){
                ui->action_log->setTextColor(Qt::red);
                ui->action_log->setText("LVDS RX UART Read Failed");
            }
        }
        else{
            rs422_freq_group[counter]->setDigitCount(5);
            rs422_glitch_group[counter]->setDigitCount(5);
            rs422_freq_group[counter]->display("-----");
            rs422_glitch_group[counter]->display("-----");

        }
        counter++;
    }
    ui->RS422_rx_read->setEnabled(true);
}

void auto_mode::on_cn_rx1_clicked(bool checked)
{
    for (QAbstractButton *button : lvds_cn_rx1_group->buttons()) {
        button->setChecked(checked);
    }
    for (QAbstractButton *button : lvds_cn_rx2_group->buttons()) {
        button->setChecked(false);
    }
    for (QAbstractButton *button : lvds_cn_rx2_tx_group->buttons()) {
        button->setChecked(checked);
    }
    for (QAbstractButton *button : lvds_cn_rx2_tx_dis_group->buttons()) {
        button->setChecked(checked);
    }
    ui->cn_rx2->setChecked(false);
    ui->rx->setChecked(false);
}

void auto_mode::on_cn_rx2_clicked(bool checked)
{
    for (QAbstractButton *button : lvds_cn_rx2_group->buttons()) {
        button->setChecked(checked);
    }
    for (QAbstractButton *button : lvds_cn_rx1_group->buttons()) {
        button->setChecked(false);
    }
    for (QAbstractButton *button : lvds_cn_rx2_tx_group->buttons()) {
        button->setChecked(checked);
    }
    for (QAbstractButton *button : lvds_cn_rx2_tx_dis_group->buttons()) {
        button->setChecked(false);
    }
    ui->cn_rx1->setChecked(false);
    ui->rx->setChecked(false);

}

void auto_mode::on_rs422_rx1_clicked(bool checked)
{

    ui->rx->setChecked(false);
    ui->rs422_rx_en_57->setChecked(checked);
    ui->rs422_rx_en_58->setChecked(false);
    ui->rs422_rx_en_59->setChecked(false);
    ui->rs422_rx_en_60->setChecked(false);
    ui->rs422_rx1->setChecked(checked);
    ui->rs422_rx2->setChecked(false);
    ui->rs422_rx3->setChecked(false);
    ui->rs422_rx4->setChecked(false);
    ui->cn_rx1->setChecked(false);
    ui->rx->setChecked(false);
}

void auto_mode::on_rs422_rx2_clicked(bool checked)
{
    ui->rx->setChecked(false);
    ui->rs422_rx_en_57->setChecked(false);
    ui->rs422_rx_en_58->setChecked(checked);
    ui->rs422_rx_en_59->setChecked(false);
    ui->rs422_rx_en_60->setChecked(false);
    ui->rs422_rx1->setChecked(false);
    ui->rs422_rx2->setChecked(checked);
    ui->rs422_rx3->setChecked(false);
    ui->rs422_rx4->setChecked(false);
    ui->cn_rx1->setChecked(false);
    ui->rx->setChecked(false);
}

void auto_mode::on_rs422_rx3_clicked(bool checked)
{
    ui->rx->setChecked(false);
    ui->rs422_rx_en_57->setChecked(false);
    ui->rs422_rx_en_58->setChecked(false);
    ui->rs422_rx_en_59->setChecked(checked);
    ui->rs422_rx_en_60->setChecked(false);
    ui->rs422_rx1->setChecked(false);
    ui->rs422_rx2->setChecked(false);
    ui->rs422_rx3->setChecked(checked);
    ui->rs422_rx4->setChecked(false);
    ui->cn_rx1->setChecked(false);
    ui->rx->setChecked(false);
}

void auto_mode::on_rs422_rx4_clicked(bool checked)
{
    ui->rx->setChecked(false);
    ui->rs422_rx_en_57->setChecked(false);
    ui->rs422_rx_en_58->setChecked(false);
    ui->rs422_rx_en_59->setChecked(false);
    ui->rs422_rx_en_60->setChecked(checked);
    ui->rs422_rx1->setChecked(false);
    ui->rs422_rx2->setChecked(false);
    ui->rs422_rx3->setChecked(false);
    ui->rs422_rx4->setChecked(checked);
    ui->cn_rx1->setChecked(false);
    ui->rx->setChecked(false);
}

