#include "auto_mode.h"
#include "ui_auto_mode.h"
#include <QMessageBox>
#include "ui_mainwindow.h"
#include "auto_mode.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <vector>
#include <cstdint>
#include <QtEndian>


uint8_t  m_header1 = 0xA7;   // 8-bit Header
bool     m_isWrite1 = true;  // 1-bit R/W flag
uint16_t m_address1 = 0x0000; // 15-bit Address (using 16-bit type)
uint16_t m_data1 = 0x0000;    // 16-bit Data
uint8_t  m_checksum1 = 0x00;


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

uint16_t gp1_grp1_live_sts_addr = 0x0100;
uint16_t gp1_grp2_live_sts_addr = 0x0101;
uint16_t gp1_grp3_live_sts_addr = 0x0102;
uint16_t gp1_grp4_live_sts_addr = 0x0103;
uint16_t gp1_grp5_live_sts_addr = 0x0104;
uint16_t gp1_grp6_live_sts_addr = 0x0105;

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
    gp1_grp1_pul_cnt_addr_lst.assign(16,0x0000);
    gp1_grp1_pul_cnt_addr_lst[0] = 0x110;
    gp1_grp1_pul_cnt_addr_lst[1] = 0x111;
    gp1_grp1_pul_cnt_addr_lst[2] = 0x112;
    gp1_grp1_pul_cnt_addr_lst[3] = 0x112;
    gp1_grp1_pul_cnt_addr_lst[4] = 0x113;
    gp1_grp1_pul_cnt_addr_lst[5] = 0x114;
    gp1_grp1_pul_cnt_addr_lst[6] = 0x115;
    gp1_grp1_pul_cnt_addr_lst[7] = 0x116;
    gp1_grp1_pul_cnt_addr_lst[8] = 0x117;
    gp1_grp1_pul_cnt_addr_lst[9] = 0x118;
    gp1_grp1_pul_cnt_addr_lst[10] = 0x11A;
    gp1_grp1_pul_cnt_addr_lst[11] = 0x11B;
    gp1_grp1_pul_cnt_addr_lst[12] = 0x11C;
    gp1_grp1_pul_cnt_addr_lst[13] = 0x11D;
    gp1_grp1_pul_cnt_addr_lst[14] = 0x11E;
    gp1_grp1_pul_cnt_addr_lst[15] = 0x11F;

    gp1_grp2_pul_cnt_addr_lst.assign(16,0x0000);
    gp1_grp2_pul_cnt_addr_lst[0] = 0x120;
    gp1_grp2_pul_cnt_addr_lst[1] = 0x121;
    gp1_grp2_pul_cnt_addr_lst[2] = 0x122;
    gp1_grp2_pul_cnt_addr_lst[3] = 0x122;
    gp1_grp2_pul_cnt_addr_lst[4] = 0x123;
    gp1_grp2_pul_cnt_addr_lst[5] = 0x124;
    gp1_grp2_pul_cnt_addr_lst[6] = 0x125;
    gp1_grp2_pul_cnt_addr_lst[7] = 0x126;
    gp1_grp2_pul_cnt_addr_lst[8] = 0x127;
    gp1_grp2_pul_cnt_addr_lst[9] = 0x128;
    gp1_grp2_pul_cnt_addr_lst[10] = 0x12A;
    gp1_grp2_pul_cnt_addr_lst[11] = 0x12B;
    gp1_grp2_pul_cnt_addr_lst[12] = 0x12C;
    gp1_grp2_pul_cnt_addr_lst[13] = 0x12D;
    gp1_grp2_pul_cnt_addr_lst[14] = 0x12E;
    gp1_grp2_pul_cnt_addr_lst[15] = 0x12F;

    gp1_grp3_pul_cnt_addr_lst.assign(16,0x0000);
    gp1_grp3_pul_cnt_addr_lst[0] = 0x130;
    gp1_grp3_pul_cnt_addr_lst[1] = 0x131;
    gp1_grp3_pul_cnt_addr_lst[2] = 0x132;
    gp1_grp3_pul_cnt_addr_lst[3] = 0x132;
    gp1_grp3_pul_cnt_addr_lst[4] = 0x133;
    gp1_grp3_pul_cnt_addr_lst[5] = 0x134;
    gp1_grp3_pul_cnt_addr_lst[6] = 0x135;
    gp1_grp3_pul_cnt_addr_lst[7] = 0x136;
    gp1_grp3_pul_cnt_addr_lst[8] = 0x137;
    gp1_grp3_pul_cnt_addr_lst[9] = 0x138;
    gp1_grp3_pul_cnt_addr_lst[10] = 0x13A;
    gp1_grp3_pul_cnt_addr_lst[11] = 0x13B;
    gp1_grp3_pul_cnt_addr_lst[12] = 0x13C;
    gp1_grp3_pul_cnt_addr_lst[13] = 0x13D;
    gp1_grp3_pul_cnt_addr_lst[14] = 0x13E;
    gp1_grp3_pul_cnt_addr_lst[15] = 0x13F;

    gp1_grp4_pul_cnt_addr_lst.assign(16,0x0000);
    gp1_grp4_pul_cnt_addr_lst[0] = 0x140;
    gp1_grp4_pul_cnt_addr_lst[1] = 0x141;
    gp1_grp4_pul_cnt_addr_lst[2] = 0x142;
    gp1_grp4_pul_cnt_addr_lst[3] = 0x142;
    gp1_grp4_pul_cnt_addr_lst[4] = 0x143;
    gp1_grp4_pul_cnt_addr_lst[5] = 0x144;
    gp1_grp4_pul_cnt_addr_lst[6] = 0x145;
    gp1_grp4_pul_cnt_addr_lst[7] = 0x146;
    gp1_grp4_pul_cnt_addr_lst[8] = 0x147;
    gp1_grp4_pul_cnt_addr_lst[9] = 0x148;
    gp1_grp4_pul_cnt_addr_lst[10] = 0x14A;
    gp1_grp4_pul_cnt_addr_lst[11] = 0x14B;
    gp1_grp4_pul_cnt_addr_lst[12] = 0x14C;
    gp1_grp4_pul_cnt_addr_lst[13] = 0x14D;
    gp1_grp4_pul_cnt_addr_lst[14] = 0x14E;
    gp1_grp4_pul_cnt_addr_lst[15] = 0x14F;

    gp1_grp5_pul_cnt_addr_lst.assign(16,0x0000);
    gp1_grp5_pul_cnt_addr_lst[0] = 0x150;
    gp1_grp5_pul_cnt_addr_lst[1] = 0x151;
    gp1_grp5_pul_cnt_addr_lst[2] = 0x152;
    gp1_grp5_pul_cnt_addr_lst[3] = 0x152;
    gp1_grp5_pul_cnt_addr_lst[4] = 0x153;
    gp1_grp5_pul_cnt_addr_lst[5] = 0x154;
    gp1_grp5_pul_cnt_addr_lst[6] = 0x155;
    gp1_grp5_pul_cnt_addr_lst[7] = 0x156;
    gp1_grp5_pul_cnt_addr_lst[8] = 0x157;
    gp1_grp5_pul_cnt_addr_lst[9] = 0x158;
    gp1_grp5_pul_cnt_addr_lst[10] = 0x15A;
    gp1_grp5_pul_cnt_addr_lst[11] = 0x15B;
    gp1_grp5_pul_cnt_addr_lst[12] = 0x15C;
    gp1_grp5_pul_cnt_addr_lst[13] = 0x15D;
    gp1_grp5_pul_cnt_addr_lst[14] = 0x15E;
    gp1_grp5_pul_cnt_addr_lst[15] = 0x15F;

    gp1_grp1_glitch_cnt_addr_lst.assign(16,0x0000);
    gp1_grp1_glitch_cnt_addr_lst[0] = 0x160;
    gp1_grp1_glitch_cnt_addr_lst[1] = 0x161;
    gp1_grp1_glitch_cnt_addr_lst[2] = 0x162;
    gp1_grp1_glitch_cnt_addr_lst[3] = 0x162;
    gp1_grp1_glitch_cnt_addr_lst[4] = 0x163;
    gp1_grp1_glitch_cnt_addr_lst[5] = 0x164;
    gp1_grp1_glitch_cnt_addr_lst[6] = 0x165;
    gp1_grp1_glitch_cnt_addr_lst[7] = 0x166;
    gp1_grp1_glitch_cnt_addr_lst[8] = 0x167;
    gp1_grp1_glitch_cnt_addr_lst[9] = 0x168;
    gp1_grp1_glitch_cnt_addr_lst[10] = 0x16A;
    gp1_grp1_glitch_cnt_addr_lst[11] = 0x16B;
    gp1_grp1_glitch_cnt_addr_lst[12] = 0x16C;
    gp1_grp1_glitch_cnt_addr_lst[13] = 0x16D;
    gp1_grp1_glitch_cnt_addr_lst[14] = 0x16E;
    gp1_grp1_glitch_cnt_addr_lst[15] = 0x16F;

    gp1_grp2_glitch_cnt_addr_lst.assign(16,0x0000);
    gp1_grp2_glitch_cnt_addr_lst[0] =   0x170;
    gp1_grp2_glitch_cnt_addr_lst[1] =   0x171;
    gp1_grp2_glitch_cnt_addr_lst[2] =   0x172;
    gp1_grp2_glitch_cnt_addr_lst[3] =   0x172;
    gp1_grp2_glitch_cnt_addr_lst[4] =   0x173;
    gp1_grp2_glitch_cnt_addr_lst[5] =   0x174;
    gp1_grp2_glitch_cnt_addr_lst[6] =   0x175;
    gp1_grp2_glitch_cnt_addr_lst[7] =   0x176;
    gp1_grp2_glitch_cnt_addr_lst[8] =   0x177;
    gp1_grp2_glitch_cnt_addr_lst[9] =   0x178;
    gp1_grp2_glitch_cnt_addr_lst[10] =  0x17A;
    gp1_grp2_glitch_cnt_addr_lst[11] =  0x17B;
    gp1_grp2_glitch_cnt_addr_lst[12] =  0x17C;
    gp1_grp2_glitch_cnt_addr_lst[13] =  0x17D;
    gp1_grp2_glitch_cnt_addr_lst[14] =  0x17E;
    gp1_grp2_glitch_cnt_addr_lst[15] =  0x17F;

    gp1_grp3_glitch_cnt_addr_lst.assign(16,0x0000);
    gp1_grp3_glitch_cnt_addr_lst[0] =   0x180;
    gp1_grp3_glitch_cnt_addr_lst[1] =   0x181;
    gp1_grp3_glitch_cnt_addr_lst[2] =   0x182;
    gp1_grp3_glitch_cnt_addr_lst[3] =   0x182;
    gp1_grp3_glitch_cnt_addr_lst[4] =   0x183;
    gp1_grp3_glitch_cnt_addr_lst[5] =   0x184;
    gp1_grp3_glitch_cnt_addr_lst[6] =   0x185;
    gp1_grp3_glitch_cnt_addr_lst[7] =   0x186;
    gp1_grp3_glitch_cnt_addr_lst[8] =   0x187;
    gp1_grp3_glitch_cnt_addr_lst[9] =   0x188;
    gp1_grp3_glitch_cnt_addr_lst[10] =  0x18A;
    gp1_grp3_glitch_cnt_addr_lst[11] =  0x18B;
    gp1_grp3_glitch_cnt_addr_lst[12] =  0x18C;
    gp1_grp3_glitch_cnt_addr_lst[13] =  0x18D;
    gp1_grp3_glitch_cnt_addr_lst[14] =  0x18E;
    gp1_grp3_glitch_cnt_addr_lst[15] =  0x18F;

    gp1_grp4_glitch_cnt_addr_lst.assign(16,0x0000);
    gp1_grp4_glitch_cnt_addr_lst[0] =   0x190;
    gp1_grp4_glitch_cnt_addr_lst[1] =   0x191;
    gp1_grp4_glitch_cnt_addr_lst[2] =   0x192;
    gp1_grp4_glitch_cnt_addr_lst[3] =   0x192;
    gp1_grp4_glitch_cnt_addr_lst[4] =   0x193;
    gp1_grp4_glitch_cnt_addr_lst[5] =   0x194;
    gp1_grp4_glitch_cnt_addr_lst[6] =   0x195;
    gp1_grp4_glitch_cnt_addr_lst[7] =   0x196;
    gp1_grp4_glitch_cnt_addr_lst[8] =   0x197;
    gp1_grp4_glitch_cnt_addr_lst[9] =   0x198;
    gp1_grp4_glitch_cnt_addr_lst[10] =  0x19A;
    gp1_grp4_glitch_cnt_addr_lst[11] =  0x19B;
    gp1_grp4_glitch_cnt_addr_lst[12] =  0x19C;
    gp1_grp4_glitch_cnt_addr_lst[13] =  0x19D;
    gp1_grp4_glitch_cnt_addr_lst[14] =  0x19E;
    gp1_grp4_glitch_cnt_addr_lst[15] =  0x19F;

    gp1_grp5_glitch_cnt_addr_lst.assign(16,0x0000);
    gp1_grp5_glitch_cnt_addr_lst[0] =   0x1a0;
    gp1_grp5_glitch_cnt_addr_lst[1] =   0x1a1;
    gp1_grp5_glitch_cnt_addr_lst[2] =   0x1a2;
    gp1_grp5_glitch_cnt_addr_lst[3] =   0x1a2;
    gp1_grp5_glitch_cnt_addr_lst[4] =   0x1a3;
    gp1_grp5_glitch_cnt_addr_lst[5] =   0x1a4;
    gp1_grp5_glitch_cnt_addr_lst[6] =   0x1a5;
    gp1_grp5_glitch_cnt_addr_lst[7] =   0x1a6;
    gp1_grp5_glitch_cnt_addr_lst[8] =   0x1a7;
    gp1_grp5_glitch_cnt_addr_lst[9] =   0x1a8;
    gp1_grp5_glitch_cnt_addr_lst[10] =  0x1aA;
    gp1_grp5_glitch_cnt_addr_lst[11] =  0x1aB;
    gp1_grp5_glitch_cnt_addr_lst[12] =  0x1aC;
    gp1_grp5_glitch_cnt_addr_lst[13] =  0x1aD;
    gp1_grp5_glitch_cnt_addr_lst[14] =  0x1aE;
    gp1_grp5_glitch_cnt_addr_lst[15] =  0x1aF;
}

auto_mode::~auto_mode()
{
    delete ui;
}

void auto_mode::on_lvds_tx_en_clicked(bool checked)
{
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
}

void auto_mode::on_cn_tx_cnfg_clicked()
{
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

    // GPO Group 3
    if(ui->lvds_tx_en_33->isChecked()){
        gpo_grp3_en_sts = gpo_grp3_en_sts | 0x1;
    }
    else {
        gpo_grp3_en_sts = gpo_grp3_en_sts & 0xFFFE;
    }
    uart_write(gpo_grp3_addr,gpo_grp3_en_sts,true);
    serial->waitForBytesWritten(10);
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
    // GPO Group 3
    if(ui->rs422_tx_en_34->isChecked()){
        gpo_grp3_en_sts = gpo_grp3_en_sts | 0x2;
    }
    else {
        gpo_grp3_en_sts = gpo_grp3_en_sts & 0xFFFD;
    }
    uart_write(gpo_grp3_addr,gpo_grp3_en_sts,true);
    serial->waitForBytesWritten(10);
}

void auto_mode::on_lvttl_tx_cnfg_clicked()
{
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
}

void auto_mode::on_lvds_rx_en_clicked(bool checked)
{
    ui->lvds_rx_en_1    ->setChecked(checked);
    ui->lvds_rx_en_2    ->setChecked(checked);
    ui->lvds_rx_en_3    ->setChecked(checked);
    ui->lvds_rx_en_4    ->setChecked(checked);
    ui->lvds_rx_en_5    ->setChecked(checked);
    ui->lvds_rx_en_6    ->setChecked(checked);
    ui->lvds_rx_en_7    ->setChecked(checked);
    ui->lvds_rx_en_8    ->setChecked(checked);
    ui->lvds_rx_en_9    ->setChecked(checked);
    ui->lvds_rx_en_10    ->setChecked(checked);
    ui->lvds_rx_en_11    ->setChecked(checked);
    ui->lvds_rx_en_12    ->setChecked(checked);
    ui->lvds_rx_en_13    ->setChecked(checked);
    ui->lvds_rx_en_14    ->setChecked(checked);
    ui->lvds_rx_en_15    ->setChecked(checked);
    ui->lvds_rx_en_16    ->setChecked(checked);
    ui->lvds_rx_en_17   ->setChecked(checked);
    ui->lvds_rx_en_18    ->setChecked(checked);
    ui->lvds_rx_en_19    ->setChecked(checked);
    ui->lvds_rx_en_20    ->setChecked(checked);
    ui->lvds_rx_en_21    ->setChecked(checked);
    ui->lvds_rx_en_22    ->setChecked(checked);
    ui->lvds_rx_en_23    ->setChecked(checked);
    ui->lvds_rx_en_24    ->setChecked(checked);
    ui->lvds_rx_en_25    ->setChecked(checked);
    ui->lvds_rx_en_26    ->setChecked(checked);
    ui->lvds_rx_en_27    ->setChecked(checked);
    ui->lvds_rx_en_28    ->setChecked(checked);
    ui->lvds_rx_en_29   ->setChecked(checked);
    ui->lvds_rx_en_30    ->setChecked(checked);
    ui->lvds_rx_en_31    ->setChecked(checked);
    ui->lvds_rx_en_32    ->setChecked(checked);
    ui->lvds_rx_en_33    ->setChecked(checked);
    ui->lvds_rx_en_34    ->setChecked(checked);
    ui->lvds_rx_en_35    ->setChecked(checked);
    ui->lvds_rx_en_36    ->setChecked(checked);
    ui->lvds_rx_en_37    ->setChecked(checked);
    ui->lvds_rx_en_38    ->setChecked(checked);
    ui->lvds_rx_en_39    ->setChecked(checked);
    ui->lvds_rx_en_40    ->setChecked(checked);
    ui->lvds_rx_en_41    ->setChecked(checked);
    ui->lvds_rx_en_42    ->setChecked(checked);
    ui->lvds_rx_en_43    ->setChecked(checked);
    ui->lvds_rx_en_44    ->setChecked(checked);
    ui->lvds_rx_en_45    ->setChecked(checked);
    ui->lvds_rx_en_46    ->setChecked(checked);
    ui->lvds_rx_en_47    ->setChecked(checked);
    ui->lvds_rx_en_48    ->setChecked(checked);
    ui->lvds_rx_en_49    ->setChecked(checked);
    ui->lvds_rx_en_50    ->setChecked(checked);
    ui->lvds_rx_en_51    ->setChecked(checked);
    ui->lvds_rx_en_52    ->setChecked(checked);
    ui->lvds_rx_en_53   ->setChecked(checked);
    ui->lvds_rx_en_54    ->setChecked(checked);
    ui->lvds_rx_en_55   ->setChecked(checked);
    ui->lvds_rx_en_56    ->setChecked(checked);
}

void auto_mode::on_rs422_rx_en_clicked(bool checked)
{
    ui->rs422_rx_en_57    ->setChecked(checked);
    ui->rs422_rx_en_58    ->setChecked(checked);
    ui->rs422_rx_en_59    ->setChecked(checked);
    ui->rs422_rx_en_60    ->setChecked(checked);
}

void auto_mode::on_lvttl_rx_en_clicked(bool checked)
{
    ui->LVTTL_rx_en_61    ->setChecked(checked);
    ui->LVTTL_rx_en_62    ->setChecked(checked);
    ui->LVTTL_rx_en_63    ->setChecked(checked);
    ui->LVTTL_rx_en_64    ->setChecked(checked);
    ui->LVTTL_rx_en_65    ->setChecked(checked);
    ui->LVTTL_rx_en_66    ->setChecked(checked);
    ui->LVTTL_rx_en_67    ->setChecked(checked);
    ui->LVTTL_rx_en_68    ->setChecked(checked);
}

void auto_mode::on_rx_clicked(bool checked)
{
    ui->lvds_rx_en      ->setChecked(checked);
    ui->rs422_rx_en      ->setChecked(checked);
    ui->lvttl_rx_en      ->setChecked(checked);
    ui->lvds_rx_en_1    ->setChecked(checked);
    ui->lvds_rx_en_2    ->setChecked(checked);
    ui->lvds_rx_en_3    ->setChecked(checked);
    ui->lvds_rx_en_4    ->setChecked(checked);
    ui->lvds_rx_en_5    ->setChecked(checked);
    ui->lvds_rx_en_6    ->setChecked(checked);
    ui->lvds_rx_en_7    ->setChecked(checked);
    ui->lvds_rx_en_8    ->setChecked(checked);
    ui->lvds_rx_en_9    ->setChecked(checked);
    ui->lvds_rx_en_10    ->setChecked(checked);
    ui->lvds_rx_en_11    ->setChecked(checked);
    ui->lvds_rx_en_12    ->setChecked(checked);
    ui->lvds_rx_en_13    ->setChecked(checked);
    ui->lvds_rx_en_14    ->setChecked(checked);
    ui->lvds_rx_en_15    ->setChecked(checked);
    ui->lvds_rx_en_16    ->setChecked(checked);
    ui->lvds_rx_en_17   ->setChecked(checked);
    ui->lvds_rx_en_18    ->setChecked(checked);
    ui->lvds_rx_en_19    ->setChecked(checked);
    ui->lvds_rx_en_20    ->setChecked(checked);
    ui->lvds_rx_en_21    ->setChecked(checked);
    ui->lvds_rx_en_22    ->setChecked(checked);
    ui->lvds_rx_en_23    ->setChecked(checked);
    ui->lvds_rx_en_24    ->setChecked(checked);
    ui->lvds_rx_en_25    ->setChecked(checked);
    ui->lvds_rx_en_26    ->setChecked(checked);
    ui->lvds_rx_en_27    ->setChecked(checked);
    ui->lvds_rx_en_28    ->setChecked(checked);
    ui->lvds_rx_en_29   ->setChecked(checked);
    ui->lvds_rx_en_30    ->setChecked(checked);
    ui->lvds_rx_en_31    ->setChecked(checked);
    ui->lvds_rx_en_32    ->setChecked(checked);
    ui->lvds_rx_en_33    ->setChecked(checked);
    ui->lvds_rx_en_34    ->setChecked(checked);
    ui->lvds_rx_en_35    ->setChecked(checked);
    ui->lvds_rx_en_36    ->setChecked(checked);
    ui->lvds_rx_en_37    ->setChecked(checked);
    ui->lvds_rx_en_38    ->setChecked(checked);
    ui->lvds_rx_en_39    ->setChecked(checked);
    ui->lvds_rx_en_40    ->setChecked(checked);
    ui->lvds_rx_en_41    ->setChecked(checked);
    ui->lvds_rx_en_42    ->setChecked(checked);
    ui->lvds_rx_en_43    ->setChecked(checked);
    ui->lvds_rx_en_44    ->setChecked(checked);
    ui->lvds_rx_en_45    ->setChecked(checked);
    ui->lvds_rx_en_46    ->setChecked(checked);
    ui->lvds_rx_en_47    ->setChecked(checked);
    ui->lvds_rx_en_48    ->setChecked(checked);
    ui->lvds_rx_en_49    ->setChecked(checked);
    ui->lvds_rx_en_50    ->setChecked(checked);
    ui->lvds_rx_en_51    ->setChecked(checked);
    ui->lvds_rx_en_52    ->setChecked(checked);
    ui->lvds_rx_en_53   ->setChecked(checked);
    ui->lvds_rx_en_54    ->setChecked(checked);
    ui->lvds_rx_en_55   ->setChecked(checked);
    ui->lvds_rx_en_56    ->setChecked(checked);
    ui->rs422_rx_en_57    ->setChecked(checked);
    ui->rs422_rx_en_58    ->setChecked(checked);
    ui->rs422_rx_en_59    ->setChecked(checked);
    ui->rs422_rx_en_60    ->setChecked(checked);
    ui->LVTTL_rx_en_61    ->setChecked(checked);
    ui->LVTTL_rx_en_62    ->setChecked(checked);
    ui->LVTTL_rx_en_63    ->setChecked(checked);
    ui->LVTTL_rx_en_64    ->setChecked(checked);
    ui->LVTTL_rx_en_65    ->setChecked(checked);
    ui->LVTTL_rx_en_66    ->setChecked(checked);
    ui->LVTTL_rx_en_67    ->setChecked(checked);
    ui->LVTTL_rx_en_68    ->setChecked(checked);
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
    // If the port is already open, the button should "Disconnect"
    if (serial->isOpen()) {
        serial->close();
        ui->connect->setText("Connect");
        ui->com_port->setEnabled(true);
        return;
    }

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
        ui->connect->setText("Disconnect");
        ui->com_port->setEnabled(false); // Lock the UI while connected
        QMessageBox::information(this, "Connected", "Successfully connected to " + portName);
    } else {
        QMessageBox::critical(this, "Port Error", "Failed to open port: " + serial->errorString());
    }
}

void auto_mode::on_read_clicked()
{
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

        if (serial->waitForReadyRead(10)) {
                QByteArray data = serial->readAll();
//                data = qToBigEndian(data);
                data = data.mid(1,2);
                ui->data_edt->setText(data.toHex(' ').toUpper());
                frame.clear();
            } else {
                ui->data_edt->setText("Timeout");
            }
        serial->flush();
    }
}

void auto_mode::on_write_clicked()
{
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

    if (bytesWritten == -1) {
        QMessageBox::critical(this, "Error", "Failed to write data: " + serial->errorString());
        frame.clear();
    } else {
        // Optional: Wait for data to be physically sent

        frame.clear();
        serial->flush();
    }
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
        uart_write(period_reg_lsb_addr,pul_prd_time,true);
        uart_write(pul_valid_addr,pul_prd_time,true);
}

void auto_mode::on_tx_enable_clicked()
{
    if(ui->tx_enable->text()== "ENABLE"){
        uart_write(pul_enable_addr,0X0001,true);
        ui->tx_enable->setText("DISABLE");
    }
    else{
        uart_write(pul_enable_addr,0X0000,true);
        ui->tx_enable->setText("ENABLE");
    }
}
