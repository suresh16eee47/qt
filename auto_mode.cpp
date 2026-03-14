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
#include <QThread>

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
    gpi_grp1_pul_cnt_addr_lst.assign(16,0x0000);
    gpi_grp1_pul_cnt_addr_lst[0] = 0x110;
    gpi_grp1_pul_cnt_addr_lst[1] = 0x111;
    gpi_grp1_pul_cnt_addr_lst[2] = 0x112;
    gpi_grp1_pul_cnt_addr_lst[3] = 0x112;
    gpi_grp1_pul_cnt_addr_lst[4] = 0x113;
    gpi_grp1_pul_cnt_addr_lst[5] = 0x114;
    gpi_grp1_pul_cnt_addr_lst[6] = 0x115;
    gpi_grp1_pul_cnt_addr_lst[7] = 0x116;
    gpi_grp1_pul_cnt_addr_lst[8] = 0x117;
    gpi_grp1_pul_cnt_addr_lst[9] = 0x118;
    gpi_grp1_pul_cnt_addr_lst[10] = 0x11A;
    gpi_grp1_pul_cnt_addr_lst[11] = 0x11B;
    gpi_grp1_pul_cnt_addr_lst[12] = 0x11C;
    gpi_grp1_pul_cnt_addr_lst[13] = 0x11D;
    gpi_grp1_pul_cnt_addr_lst[14] = 0x11E;
    gpi_grp1_pul_cnt_addr_lst[15] = 0x11F;

    gpi_grp2_pul_cnt_addr_lst.assign(16,0x0000);
    gpi_grp2_pul_cnt_addr_lst[0] = 0x120;
    gpi_grp2_pul_cnt_addr_lst[1] = 0x121;
    gpi_grp2_pul_cnt_addr_lst[2] = 0x122;
    gpi_grp2_pul_cnt_addr_lst[3] = 0x122;
    gpi_grp2_pul_cnt_addr_lst[4] = 0x123;
    gpi_grp2_pul_cnt_addr_lst[5] = 0x124;
    gpi_grp2_pul_cnt_addr_lst[6] = 0x125;
    gpi_grp2_pul_cnt_addr_lst[7] = 0x126;
    gpi_grp2_pul_cnt_addr_lst[8] = 0x127;
    gpi_grp2_pul_cnt_addr_lst[9] = 0x128;
    gpi_grp2_pul_cnt_addr_lst[10] = 0x12A;
    gpi_grp2_pul_cnt_addr_lst[11] = 0x12B;
    gpi_grp2_pul_cnt_addr_lst[12] = 0x12C;
    gpi_grp2_pul_cnt_addr_lst[13] = 0x12D;
    gpi_grp2_pul_cnt_addr_lst[14] = 0x12E;
    gpi_grp2_pul_cnt_addr_lst[15] = 0x12F;

    gpi_grp3_pul_cnt_addr_lst.assign(16,0x0000);
    gpi_grp3_pul_cnt_addr_lst[0] = 0x130;
    gpi_grp3_pul_cnt_addr_lst[1] = 0x131;
    gpi_grp3_pul_cnt_addr_lst[2] = 0x132;
    gpi_grp3_pul_cnt_addr_lst[3] = 0x132;
    gpi_grp3_pul_cnt_addr_lst[4] = 0x133;
    gpi_grp3_pul_cnt_addr_lst[5] = 0x134;
    gpi_grp3_pul_cnt_addr_lst[6] = 0x135;
    gpi_grp3_pul_cnt_addr_lst[7] = 0x136;
    gpi_grp3_pul_cnt_addr_lst[8] = 0x137;
    gpi_grp3_pul_cnt_addr_lst[9] = 0x138;
    gpi_grp3_pul_cnt_addr_lst[10] = 0x13A;
    gpi_grp3_pul_cnt_addr_lst[11] = 0x13B;
    gpi_grp3_pul_cnt_addr_lst[12] = 0x13C;
    gpi_grp3_pul_cnt_addr_lst[13] = 0x13D;
    gpi_grp3_pul_cnt_addr_lst[14] = 0x13E;
    gpi_grp3_pul_cnt_addr_lst[15] = 0x13F;

    gpi_grp4_pul_cnt_addr_lst.assign(16,0x0000);
    gpi_grp4_pul_cnt_addr_lst[0] = 0x140;
    gpi_grp4_pul_cnt_addr_lst[1] = 0x141;
    gpi_grp4_pul_cnt_addr_lst[2] = 0x142;
    gpi_grp4_pul_cnt_addr_lst[3] = 0x142;
    gpi_grp4_pul_cnt_addr_lst[4] = 0x143;
    gpi_grp4_pul_cnt_addr_lst[5] = 0x144;
    gpi_grp4_pul_cnt_addr_lst[6] = 0x145;
    gpi_grp4_pul_cnt_addr_lst[7] = 0x146;
    gpi_grp4_pul_cnt_addr_lst[8] = 0x147;
    gpi_grp4_pul_cnt_addr_lst[9] = 0x148;
    gpi_grp4_pul_cnt_addr_lst[10] = 0x14A;
    gpi_grp4_pul_cnt_addr_lst[11] = 0x14B;
    gpi_grp4_pul_cnt_addr_lst[12] = 0x14C;
    gpi_grp4_pul_cnt_addr_lst[13] = 0x14D;
    gpi_grp4_pul_cnt_addr_lst[14] = 0x14E;
    gpi_grp4_pul_cnt_addr_lst[15] = 0x14F;

    gpi_grp5_pul_cnt_addr_lst.assign(16,0x0000);
    gpi_grp5_pul_cnt_addr_lst[0] = 0x150;
    gpi_grp5_pul_cnt_addr_lst[1] = 0x151;
    gpi_grp5_pul_cnt_addr_lst[2] = 0x152;
    gpi_grp5_pul_cnt_addr_lst[3] = 0x152;
    gpi_grp5_pul_cnt_addr_lst[4] = 0x153;
    gpi_grp5_pul_cnt_addr_lst[5] = 0x154;
    gpi_grp5_pul_cnt_addr_lst[6] = 0x155;
    gpi_grp5_pul_cnt_addr_lst[7] = 0x156;
    gpi_grp5_pul_cnt_addr_lst[8] = 0x157;
    gpi_grp5_pul_cnt_addr_lst[9] = 0x158;
    gpi_grp5_pul_cnt_addr_lst[10] = 0x15A;
    gpi_grp5_pul_cnt_addr_lst[11] = 0x15B;
    gpi_grp5_pul_cnt_addr_lst[12] = 0x15C;
    gpi_grp5_pul_cnt_addr_lst[13] = 0x15D;
    gpi_grp5_pul_cnt_addr_lst[14] = 0x15E;
    gpi_grp5_pul_cnt_addr_lst[15] = 0x15F;

    gpi_grp1_glitch_cnt_addr_lst.assign(16,0x0000);
    gpi_grp1_glitch_cnt_addr_lst[0] = 0x160;
    gpi_grp1_glitch_cnt_addr_lst[1] = 0x161;
    gpi_grp1_glitch_cnt_addr_lst[2] = 0x162;
    gpi_grp1_glitch_cnt_addr_lst[3] = 0x162;
    gpi_grp1_glitch_cnt_addr_lst[4] = 0x163;
    gpi_grp1_glitch_cnt_addr_lst[5] = 0x164;
    gpi_grp1_glitch_cnt_addr_lst[6] = 0x165;
    gpi_grp1_glitch_cnt_addr_lst[7] = 0x166;
    gpi_grp1_glitch_cnt_addr_lst[8] = 0x167;
    gpi_grp1_glitch_cnt_addr_lst[9] = 0x168;
    gpi_grp1_glitch_cnt_addr_lst[10] = 0x16A;
    gpi_grp1_glitch_cnt_addr_lst[11] = 0x16B;
    gpi_grp1_glitch_cnt_addr_lst[12] = 0x16C;
    gpi_grp1_glitch_cnt_addr_lst[13] = 0x16D;
    gpi_grp1_glitch_cnt_addr_lst[14] = 0x16E;
    gpi_grp1_glitch_cnt_addr_lst[15] = 0x16F;

    gpi_grp2_glitch_cnt_addr_lst.assign(16,0x0000);
    gpi_grp2_glitch_cnt_addr_lst[0] =   0x170;
    gpi_grp2_glitch_cnt_addr_lst[1] =   0x171;
    gpi_grp2_glitch_cnt_addr_lst[2] =   0x172;
    gpi_grp2_glitch_cnt_addr_lst[3] =   0x172;
    gpi_grp2_glitch_cnt_addr_lst[4] =   0x173;
    gpi_grp2_glitch_cnt_addr_lst[5] =   0x174;
    gpi_grp2_glitch_cnt_addr_lst[6] =   0x175;
    gpi_grp2_glitch_cnt_addr_lst[7] =   0x176;
    gpi_grp2_glitch_cnt_addr_lst[8] =   0x177;
    gpi_grp2_glitch_cnt_addr_lst[9] =   0x178;
    gpi_grp2_glitch_cnt_addr_lst[10] =  0x17A;
    gpi_grp2_glitch_cnt_addr_lst[11] =  0x17B;
    gpi_grp2_glitch_cnt_addr_lst[12] =  0x17C;
    gpi_grp2_glitch_cnt_addr_lst[13] =  0x17D;
    gpi_grp2_glitch_cnt_addr_lst[14] =  0x17E;
    gpi_grp2_glitch_cnt_addr_lst[15] =  0x17F;

    gpi_grp3_glitch_cnt_addr_lst.assign(16,0x0000);
    gpi_grp3_glitch_cnt_addr_lst[0] =   0x180;
    gpi_grp3_glitch_cnt_addr_lst[1] =   0x181;
    gpi_grp3_glitch_cnt_addr_lst[2] =   0x182;
    gpi_grp3_glitch_cnt_addr_lst[3] =   0x182;
    gpi_grp3_glitch_cnt_addr_lst[4] =   0x183;
    gpi_grp3_glitch_cnt_addr_lst[5] =   0x184;
    gpi_grp3_glitch_cnt_addr_lst[6] =   0x185;
    gpi_grp3_glitch_cnt_addr_lst[7] =   0x186;
    gpi_grp3_glitch_cnt_addr_lst[8] =   0x187;
    gpi_grp3_glitch_cnt_addr_lst[9] =   0x188;
    gpi_grp3_glitch_cnt_addr_lst[10] =  0x18A;
    gpi_grp3_glitch_cnt_addr_lst[11] =  0x18B;
    gpi_grp3_glitch_cnt_addr_lst[12] =  0x18C;
    gpi_grp3_glitch_cnt_addr_lst[13] =  0x18D;
    gpi_grp3_glitch_cnt_addr_lst[14] =  0x18E;
    gpi_grp3_glitch_cnt_addr_lst[15] =  0x18F;

    gpi_grp4_glitch_cnt_addr_lst.assign(16,0x0000);
    gpi_grp4_glitch_cnt_addr_lst[0] =   0x190;
    gpi_grp4_glitch_cnt_addr_lst[1] =   0x191;
    gpi_grp4_glitch_cnt_addr_lst[2] =   0x192;
    gpi_grp4_glitch_cnt_addr_lst[3] =   0x192;
    gpi_grp4_glitch_cnt_addr_lst[4] =   0x193;
    gpi_grp4_glitch_cnt_addr_lst[5] =   0x194;
    gpi_grp4_glitch_cnt_addr_lst[6] =   0x195;
    gpi_grp4_glitch_cnt_addr_lst[7] =   0x196;
    gpi_grp4_glitch_cnt_addr_lst[8] =   0x197;
    gpi_grp4_glitch_cnt_addr_lst[9] =   0x198;
    gpi_grp4_glitch_cnt_addr_lst[10] =  0x19A;
    gpi_grp4_glitch_cnt_addr_lst[11] =  0x19B;
    gpi_grp4_glitch_cnt_addr_lst[12] =  0x19C;
    gpi_grp4_glitch_cnt_addr_lst[13] =  0x19D;
    gpi_grp4_glitch_cnt_addr_lst[14] =  0x19E;
    gpi_grp4_glitch_cnt_addr_lst[15] =  0x19F;

    gpi_grp5_glitch_cnt_addr_lst.assign(16,0x0000);
    gpi_grp5_glitch_cnt_addr_lst[0] =   0x1a0;
    gpi_grp5_glitch_cnt_addr_lst[1] =   0x1a1;
    gpi_grp5_glitch_cnt_addr_lst[2] =   0x1a2;
    gpi_grp5_glitch_cnt_addr_lst[3] =   0x1a2;
    gpi_grp5_glitch_cnt_addr_lst[4] =   0x1a3;
    gpi_grp5_glitch_cnt_addr_lst[5] =   0x1a4;
    gpi_grp5_glitch_cnt_addr_lst[6] =   0x1a5;
    gpi_grp5_glitch_cnt_addr_lst[7] =   0x1a6;
    gpi_grp5_glitch_cnt_addr_lst[8] =   0x1a7;
    gpi_grp5_glitch_cnt_addr_lst[9] =   0x1a8;
    gpi_grp5_glitch_cnt_addr_lst[10] =  0x1aA;
    gpi_grp5_glitch_cnt_addr_lst[11] =  0x1aB;
    gpi_grp5_glitch_cnt_addr_lst[12] =  0x1aC;
    gpi_grp5_glitch_cnt_addr_lst[13] =  0x1aD;
    gpi_grp5_glitch_cnt_addr_lst[14] =  0x1aE;
    gpi_grp5_glitch_cnt_addr_lst[15] =  0x1aF;
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

            if (serial->waitForReadyRead(50)) {
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
    if(ui->lvds_rx_en_1->isChecked()){
        freq = uart_read(gpi_grp1_pul_cnt_addr_lst[0]);
        glitch_cnt = uart_read(gpi_grp1_glitch_cnt_addr_lst[0]);
        if(freq != 0xB055){
            ui->lvds_freq_1->setDigitCount(5);
            ui->lvds_glitch_cnt_1->setDigitCount(5);
            ui->lvds_freq_1->display(freq);
            ui->lvds_glitch_cnt_1->display(glitch_cnt);
        }
    }
    else if(ui->lvds_rx_en_2->isChecked()){
        freq = uart_read(gpi_grp1_pul_cnt_addr_lst[1]);
        glitch_cnt = uart_read(gpi_grp1_glitch_cnt_addr_lst[1]);
        if(freq != 0xB055){
            ui->lvds_freq_1->setDigitCount(5);
            ui->lvds_glitch_cnt_1->setDigitCount(5);
            ui->lvds_freq_1->display(freq);
            ui->lvds_glitch_cnt_1->display(glitch_cnt);
        }
    }
    else if(ui->lvds_rx_en_3->isChecked()){
        freq = uart_read(gpi_grp1_pul_cnt_addr_lst[2]);
        glitch_cnt = uart_read(gpi_grp1_glitch_cnt_addr_lst[2]);
        if(freq != 0xB055){
            ui->lvds_freq_1->setDigitCount(5);
            ui->lvds_glitch_cnt_1->setDigitCount(5);
            ui->lvds_freq_1->display(freq);
            ui->lvds_glitch_cnt_1->display(glitch_cnt);
        }
    }
    else if(ui->lvds_rx_en_4->isChecked()){
        freq = uart_read(gpi_grp1_pul_cnt_addr_lst[3]);
        glitch_cnt = uart_read(gpi_grp1_glitch_cnt_addr_lst[3]);
        if(freq != 0xB055){
            ui->lvds_freq_1->setDigitCount(5);
            ui->lvds_glitch_cnt_1->setDigitCount(5);
            ui->lvds_freq_1->display(freq);
            ui->lvds_glitch_cnt_1->display(glitch_cnt);
        }
    }
    else if(ui->lvds_rx_en_5->isChecked()){
        freq = uart_read(gpi_grp1_pul_cnt_addr_lst[4]);
        glitch_cnt = uart_read(gpi_grp1_glitch_cnt_addr_lst[4]);
        if(freq != 0xB055){
            ui->lvds_freq_1->setDigitCount(5);
            ui->lvds_glitch_cnt_1->setDigitCount(5);
            ui->lvds_freq_1->display(freq);
            ui->lvds_glitch_cnt_1->display(glitch_cnt);
        }
    }
    else if(ui->lvds_rx_en_6->isChecked()){
        freq = uart_read(gpi_grp1_pul_cnt_addr_lst[5]);
        glitch_cnt = uart_read(gpi_grp1_glitch_cnt_addr_lst[5]);
        if(freq != 0xB055){
            ui->lvds_freq_1->setDigitCount(5);
            ui->lvds_glitch_cnt_1->setDigitCount(5);
            ui->lvds_freq_1->display(freq);
            ui->lvds_glitch_cnt_1->display(glitch_cnt);
        }
    }
    if(ui->lvds_rx_en_18->isChecked()){
        freq = uart_read(gpi_grp2_pul_cnt_addr_lst[14]);
        glitch_cnt = uart_read(gpi_grp2_glitch_cnt_addr_lst[14]);
        if(freq != 0xB055){
            ui->lvds_freq_18->setDigitCount(5);
            ui->lvds_glitch_cnt_18->setDigitCount(5);
            ui->lvds_freq_18->display(freq);
            ui->lvds_glitch_cnt_18->display(glitch_cnt);
        }
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
    QPalette pal = ui->rx_freq_61->palette();

    quint16 freq = 0;
//    quint16 glitch_cnt = 0;
    if(ui->LVTTL_rx_en_61->isChecked()){
        freq = uart_read(gpi_grp4_pul_cnt_addr_lst[5]);
//        glitch_cnt = uart_read(gpi_grp3_glitch_cnt_addr_lst[5]);
        if(freq != 0xB055){
            if(freq == 0x03){
                pal.setColor(QPalette::WindowText, Qt::green);
                // Set background color to black
                pal.setColor(QPalette::Window, Qt::black);
    //            ui->lvds_glitch_cnt_18->display(glitch_cnt);
                ui->rx_freq_61->setDigitCount(4);
    //            ui->lvds_glitch_cnt_18->setDigitCount(5);
                ui->rx_freq_61->display("PASS");
                // Set digit color to green
            }
            else{
                pal.setColor(QPalette::WindowText, Qt::red);
                // Set background color to black
                pal.setColor(QPalette::Window, Qt::black);
                ui->rx_freq_61->setDigitCount(4);
    //            ui->lvds_glitch_cnt_18->setDigitCount(5);
                ui->rx_freq_61->display("FAIL");
    //            ui->lvds_glitch_cnt_18->display(glitch_cnt);


            }
        }
    }
    ui->lvds_rx_read->setEnabled(true);
}
