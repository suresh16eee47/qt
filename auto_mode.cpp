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
#include <QIntValidator>
#include <QDateTime>
#include <QValidator>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

void appendToFile(QString text) {
    // 1. Specify the file path
    QFile file("log.txt");

    // 2. Open in Append and WriteOnly mode
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);

        // 3. Write the string (adding a newline if needed)
        out << text << "\n";

        file.close();
    }
}

class HexRangeValidator : public QValidator {
public:
    HexRangeValidator(int min, int max, QObject *parent = nullptr)
        : QValidator(parent), m_min(min), m_max(max) {}

    State validate(QString &input, int &pos) const override {
        if (input.isEmpty()) return Intermediate;

        bool ok;
        int value = input.toInt(&ok, 16);

        if (!ok) return Invalid; // Not a hex number
        if (value > m_max) return Invalid; // Exceeds FF

        // Allow "0" as Intermediate so user can type "01",
        // but it won't be "Acceptable" until it's within 1-FF.
        if (value < m_min) return Intermediate;

        return Acceptable;
    }

private:
    int m_min;
    int m_max;
};

uint8_t  m_header1 = 0xA7;   // 8-bit Header
uint8_t  bulk_header1 = 0xA1; // Bulk read and write header
bool     m_isWrite1 = true;  // 1-bit R/W flag
uint16_t m_address1 = 0x0000; // 15-bit Address (using 16-bit type)
uint16_t m_data1 = 0x0000;    // 16-bit Data
uint8_t  m_checksum1 = 0x00;
QByteArray status = 0;


uint16_t on_pul_reg_addr        = 0x0108;
uint16_t period_reg_lsb_addr    = 0x0109;
uint16_t period_reg_msb_addr    = 0x010A;
uint16_t pul_valid_addr         = 0x010E;
uint16_t pul_enable_addr        = 0x010C;

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


uint16_t flash_inst_reg = 0x0200;
uint16_t flash_write_reg = 0x0201;
uint16_t flash_lsb_addr_reg = 0x0202;
uint16_t flash_msb_addr_reg = 0x0203;
uint16_t flash_data_cnt_reg = 0x0204;
uint16_t flash_fsm_sts_reg = 0x0205;
uint16_t flash_read_fifo_cnt_reg = 0x0206;
uint16_t flash_read_fifo_data_reg = 0x0207;
uint16_t flash_read_data_reg = 0x0208;
bool flash_erase_sts = false;
bool flash_mode = false;

bool cont_mode = false;
uint16_t cont_cntr = 0;

bool debug_visibility = false;

auto_mode::auto_mode(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::auto_mode)
{
    ui->setupUi(this);
    serial = new QSerialPort(this);
    lvds_rx_group = new QButtonGroup(this);
    bool ui_enable = false;
    QRegularExpression rx("^([1-9]|10)$");
    QRegularExpressionValidator *freq_valid = new QRegularExpressionValidator(rx,this);
    ui->freq_ip->setValidator(freq_valid);
    ui->Transmitter_group->setEnabled(ui_enable);
    ui->Receiver_group->setEnabled(ui_enable);
    ui->Debug_group->setEnabled(ui_enable);
    ui->Debug_group->setVisible(true);
    ui->test_mode->setVisible(false);
    ui->flash_group->setVisible(false);
    ui->flash_group_1->setVisible(false);
    ui->uart_bulk_r->setVisible(false);
    ui->rs422_tx_en->setVisible(false);
    ui->rx->setVisible(false);
    this->setWindowTitle("DP-SPL-8302-636");

    // Usage:
    auto *flash_address_edt_val = new HexRangeValidator(1, 33554431, this); // 1 to 0xFF
    ui->flash_address_edt->setValidator(flash_address_edt_val);
    auto *flash_data_edt_val = new HexRangeValidator(1,255, this); // 1 to 0xFF
    ui->flash_address_edt->setValidator(flash_data_edt_val);
    auto *address_edt_val = new HexRangeValidator(1,65535, this); // 1 to 0xFF
    ui->flash_address_edt->setValidator(address_edt_val);
    auto *data_edt_val = new HexRangeValidator(1,65535, this); // 1 to 0xFF
    ui->flash_address_edt->setValidator(data_edt_val);


    ui->read->setText("REA&D");
    ui->tx_config->setText("&CONFIGURE");
    ui->tx_enable->setText("&ENABLE");
    ui->read_all->setText("&READ ALL");
    ui->lvds_rx_en->setText("L&VDS RX");
    ui->rs422_rx_en->setText("R&S422 RX");
    ui->lvttl_rx_en->setText("LV&TTL RX");
    ui->connect->setText("C&ONNECT");
    ui->cn_rx1->setText("CN-RX&1");
    ui->cn_rx2->setText("CN-RX&2");
    ui->rs422_rx3->setText("RS422-RX&3");
    ui->rs422_rx4->setText("RS422-RX&4");

    // 1. Enable background filling
    ui->lvds_freq_35->setAutoFillBackground(true);


    QPixmap pixmap(":/Res/Refresh.png");
    if (pixmap.isNull()) {
        action_log(QString("Error: Image not found at path!"),Qt::red);
    } else {
        ui->ref->setIcon(QIcon(pixmap));
        ui->ref->setIconSize(QSize(32, 32));
    }

    // 2. Modify the palette
    QColor ashGray("#B2BEB5");
    QPalette pal = ui->lvds_freq_35->palette();
    pal.setColor(QPalette::WindowText, Qt::black);  // Digit color
    pal.setColor(QPalette::Background, Qt::green);



    // tool tip
    uint16_t tip_duration = 10000;
    //LVDS RX CHANNEL 1
    ui->lvds_rx_en_35->setToolTip("LVTTL-TRM-21-RX (0x13D)");
    ui->lvds_rx_en_35->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 2
    ui->lvds_rx_en_52->setToolTip("LVTTL-TRM-25-SOB (0x14C)");
    ui->lvds_rx_en_52->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 3
    ui->lvds_rx_en_26->setToolTip("LVTTL-TRM-12-RX (0x126)");
    ui->lvds_rx_en_26->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 4
    ui->lvds_rx_en_31->setToolTip("LVTTL-TRM-17-RX (0x121)");
    ui->lvds_rx_en_31->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 5
    ui->lvds_rx_en_25->setToolTip("LVTTL-TRM-11-RX (0x127)");
    ui->lvds_rx_en_25->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 6
    ui->lvds_rx_en_30->setToolTip("LVTTL-TRM-16-RX (0x122)");
    ui->lvds_rx_en_30->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 7
    ui->lvds_rx_en_49->setToolTip("LVTTL-TRM-24-TRP (0x14F)");
    ui->lvds_rx_en_49->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 8
    ui->lvds_rx_en_46->setToolTip("LVTTL-TRM-23-TX (0x132)");
    ui->lvds_rx_en_46->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 9
    ui->lvds_rx_en_28->setToolTip("LVTTL-TRM-14-RX (0x124)");
    ui->lvds_rx_en_28->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 10
    ui->lvds_rx_en_54->setToolTip("LVTTL-TRM-25-TX (0x14A)");
    ui->lvds_rx_en_54->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 11
    ui->lvds_rx_en_50->setToolTip("LVTTL-TRM-24-TX (0x14E)");
    ui->lvds_rx_en_50->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 12
    ui->lvds_rx_en_40->setToolTip("LVTTL-TRM-22-SOB (0x138)");
    ui->lvds_rx_en_40->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 13
    ui->lvds_rx_en_48->setToolTip("LVTTL-TRM-24-SOB (0x130)");
    ui->lvds_rx_en_48->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 14
    ui->lvds_rx_en_45->setToolTip("LVTTL-TRM-23-TRP (0x133)");
    ui->lvds_rx_en_45->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 15
    ui->lvds_rx_en_53->setToolTip("LVTTL-TRM-25-TRP (0x14B)");
    ui->lvds_rx_en_53->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 16
    ui->lvds_rx_en_41->setToolTip("LVTTL-TRM-22-TRP (0x137)");
    ui->lvds_rx_en_41->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 17
    ui->lvds_rx_en_38->setToolTip("LVTTL-TRM-21-TX (0x13A)");
    ui->lvds_rx_en_38->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 18
    ui->lvds_rx_en_43->setToolTip("LVTTL-TRM-23-RX (0x135)");
    ui->lvds_rx_en_43->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 19
    ui->lvds_rx_en_6->setToolTip("LVTTL-TRM-2-SOB (0x11A)");
    ui->lvds_rx_en_6->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 20
    ui->lvds_rx_en_44->setToolTip("LVTTL-TRM-23-SOB (0x134)");
    ui->lvds_rx_en_44->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 21
    ui->lvds_rx_en_8->setToolTip("LVTTL-TRM-2-TX (0x118)");
    ui->lvds_rx_en_8->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 22
    ui->lvds_rx_en_11->setToolTip("LVTTL-TRM-3-TRP (0x115)");
    ui->lvds_rx_en_11->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 23
    ui->lvds_rx_en_7->setToolTip("LVTTL-TRM-2-TRP (0x119)");
    ui->lvds_rx_en_7->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 24
    ui->lvds_rx_en_16->setToolTip("LVTTL-TRM-4-TX (0x110)");
    ui->lvds_rx_en_16->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 25
    ui->lvds_rx_en_20->setToolTip("LVTTL-TRM-6-RX (0x12C)");
    ui->lvds_rx_en_20->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 26
    ui->lvds_rx_en_14->setToolTip("LVTTL-TRM-4-SOB (0x112)");
    ui->lvds_rx_en_14->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 27
    ui->lvds_rx_en_17->setToolTip("LVTTL-TRM-5-SOB (0x12F)");
    ui->lvds_rx_en_17->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 28
    ui->lvds_rx_en_18->setToolTip("LVTTL-TRM-5-TRP (0x12E)");
    ui->lvds_rx_en_18->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 29
    ui->lvds_rx_en_1->setToolTip("LVTTL-TRM-1-RX (0x11F)");
    ui->lvds_rx_en_1->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 30
    ui->lvds_rx_en_3->setToolTip("LVTTL-TRM-1-TRP (0x11D)");
    ui->lvds_rx_en_3->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 31
    ui->lvds_rx_en_10->setToolTip("LVTTL-TRM-3-SOB (0x116)");
    ui->lvds_rx_en_10->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 32
    ui->lvds_rx_en_4->setToolTip("LVTTL-TRM-1-TX (0x11C)");
    ui->lvds_rx_en_4->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 33
    ui->lvds_rx_en_19->setToolTip("LVTTL-TRM-5-TX (0x12D)");
    ui->lvds_rx_en_19->setToolTipDuration(tip_duration);

    //CN-RX2
    //LVDS RX CHANNEL 34
    ui->lvds_rx_en_33->setToolTip("LVTTL-TRM-19-RX (0x13F)");
    ui->lvds_rx_en_33->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 35
    ui->lvds_rx_en_29->setToolTip("LVTTL-TRM-15-RX (0x123)");
    ui->lvds_rx_en_29->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 36
    ui->lvds_rx_en_27->setToolTip("LVTTL-TRM-13-RX (0x125)");
    ui->lvds_rx_en_27->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 37
    ui->lvds_rx_en_34->setToolTip("LVTTL-TRM-20-RX (0x13E)");
    ui->lvds_rx_en_34->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 38
    ui->lvds_rx_en_47->setToolTip("LVTTL-TRM-24-RX (0x131)");
    ui->lvds_rx_en_47->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 39
    ui->lvds_rx_en_39->setToolTip("LVTTL-TRM-22-RX (0x139)");
    ui->lvds_rx_en_39->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 40
    ui->lvds_rx_en_55->setToolTip("LVTTL-TRM-26-RX (0x149)");
    ui->lvds_rx_en_55->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 41
    ui->lvds_rx_en_42->setToolTip("LVTTL-TRM-22-TX (0x136)");
    ui->lvds_rx_en_42->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 42
    ui->lvds_rx_en_51->setToolTip("LVTTL-TRM-25-RX (0x14D)");
    ui->lvds_rx_en_51->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 43
    ui->lvds_rx_en_36->setToolTip("LVTTL-TRM-21-SOB (0x13C)");
    ui->lvds_rx_en_36->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 44
    ui->lvds_rx_en_24->setToolTip("LVTTL-TRM-10-RX (0x128)");
    ui->lvds_rx_en_24->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 45
    ui->lvds_rx_en_15->setToolTip("LVTTL-TRM-4-TRP (0x111)");
    ui->lvds_rx_en_15->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 46
    ui->lvds_rx_en_9->setToolTip("LVTTL-TRM-3-RX (0x117)");
    ui->lvds_rx_en_9->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 47
    ui->lvds_rx_en_2->setToolTip("LVTTL-TRM-1-SOB (0x11E)");
    ui->lvds_rx_en_2->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 48
    ui->lvds_rx_en_23->setToolTip("LVTTL-TRM-9-RX (0x129)");
    ui->lvds_rx_en_23->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 49
    ui->lvds_rx_en_21->setToolTip("LVTTL-TRM-7-RX (0x12B)");
    ui->lvds_rx_en_21->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 50
    ui->lvds_rx_en_12->setToolTip("LVTTL-TRM-3-TX (0x114)");
    ui->lvds_rx_en_12->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 51
    ui->lvds_rx_en_22->setToolTip("LVTTL-TRM-8-RX (0x12A)");
    ui->lvds_rx_en_22->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 52
    ui->lvds_rx_en_13->setToolTip("LVTTL-TRM-4-RX (0x113)");
    ui->lvds_rx_en_13->setToolTipDuration(tip_duration);
    //LVDS RX CHANNEL 53
    ui->lvds_rx_en_5->setToolTip("LVTTL-TRM-2-RX (0x11B)");
    ui->lvds_rx_en_5->setToolTipDuration(tip_duration);









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
    lvds_cn_rx2_group ->addButton(ui->lvds_rx_en_56);
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

    gpi_lvds_pul_sts_cnt_addr_lst.assign(56,0x0000);
    gpi_rs422_pul_sts_cnt_addr_lst.assign(4,0x0000);
    gpi_lvttl_pul_sts_cnt_addr_lst.assign(8,0x0000);

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
    if(ui->cn_rx2->isChecked()){
        for (QAbstractButton *button : lvds_cn_rx2_tx_group->buttons()) {
            button->setChecked(checked);
        }
        for (QAbstractButton *button : lvds_cn_rx2_tx_dis_group->buttons()) {
            button->setChecked(false);
        }
    }
    else
    {
        for (QAbstractButton *button : lvds_cn_rx2_tx_group->buttons()) {
            button->setChecked(checked);
        }
        for (QAbstractButton *button : lvds_cn_rx2_tx_dis_group->buttons()) {
            button->setChecked(checked);
        }
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
    bool wr_sts = uart_write(gpo_grp1_addr,gpo_grp1_en_sts,true);
    serial->waitForBytesWritten(10);
    serial->waitForReadyRead(50);
    status = serial->readAll();
    if(wr_sts == true){
//        action_log("TX Group 1 configuration success",Qt::green);
    }
    else{
        action_log("TX Group 1 configuration failed",Qt::red);
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
    wr_sts = uart_write(gpo_grp2_addr,gpo_grp2_en_sts,true);
    serial->waitForBytesWritten(10);
    serial->waitForReadyRead(50);
    status = serial->readAll();
    if(wr_sts == true){
//        action_log("TX Group 2 configuration success",Qt::green);
    }
    else{
        action_log("TX Group 2 configuration failed",Qt::red);
    }

    // GPO Group 3
    if(ui->lvds_tx_en_33->isChecked()){
        gpo_grp3_en_sts = gpo_grp3_en_sts | 0x1;
    }
    else {
        gpo_grp3_en_sts = gpo_grp3_en_sts & 0xFFFE;
    }
    wr_sts = uart_write(gpo_grp3_addr,gpo_grp3_en_sts,true);
    serial->waitForBytesWritten(10);
    serial->waitForReadyRead(50);
    status = serial->readAll();
    if(wr_sts == true){
//        action_log("TX Group 3 configuration success",Qt::green);
    }
    else{
        action_log("TX Group 3 configuration failed",Qt::red);
    }
    if(wr_sts == true){
        action_log("LVDS TX Configuration Success",Qt::green);
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
    if(ui->cn_rx2->isChecked()){
        for (QAbstractButton *button : lvds_cn_rx2_tx_group->buttons()) {
            button->setChecked(checked);
        }
        for (QAbstractButton *button : lvds_cn_rx2_tx_dis_group->buttons()) {
            button->setChecked(false);
        }
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
        ui->lvds_tx_en->setChecked(checked);
    }
    else if(!ui->cn_rx2->isChecked()){
        for (QAbstractButton *button : lvds_cn_rx2_tx_group->buttons()) {
            button->setChecked(checked);
        }
        for (QAbstractButton *button : lvds_cn_rx2_tx_dis_group->buttons()) {
            button->setChecked(checked);
        }
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
        ui->lvds_tx_en->setChecked(checked);
    }





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
    bool wr_sts = uart_write(gpo_grp3_addr,gpo_grp3_en_sts,true);
    serial->waitForBytesWritten(10);
    serial->waitForReadyRead(50);
    if(wr_sts == true){
        action_log("RS422 TX configuration success",Qt::green);
    }
    else{
        action_log("RS422 TX configuration Failed",Qt::red);
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
    bool wr_sts = uart_write(gpo_grp3_addr,gpo_grp3_en_sts,true);
    serial->waitForBytesWritten(10);
    serial->waitForReadyRead(50);
    if(wr_sts == true){
        action_log("LVTTL TX Configuration Success",Qt::green);
    }
    else{
        action_log("LVTTL TX Configuration Failed",Qt::red);
    }
    ui->lvttl_tx_cnfg->setEnabled(true);
}

void auto_mode::on_lvds_rx_en_clicked(bool checked)
{
    if(ui->cn_rx1->isChecked()){
        for (QAbstractButton *button : lvds_cn_rx1_group->buttons()) {
                button->setChecked(checked);
            }
        for (QAbstractButton *button : lvds_cn_rx2_group->buttons()) {
                button->setChecked(false);
            }
    }
    else if(ui->cn_rx2->isChecked())
    {
        for (QAbstractButton *button : lvds_cn_rx1_group->buttons()) {
                button->setChecked(false);
            }
        for (QAbstractButton *button : lvds_cn_rx2_group->buttons()) {
                button->setChecked(checked);
            }
    }
    else{
        for (QAbstractButton *button : lvds_cn_rx1_group->buttons()) {
                button->setChecked(checked);
            }
        for (QAbstractButton *button : lvds_cn_rx2_group->buttons()) {
                button->setChecked(checked);
            }
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
    if(ui->connect->text() == "C&ONNECT"){
        // 1. Set the Port Name from the Combo Box
        QString portName = ui->com_port->currentText();
        if (portName.isEmpty()) {
            action_log(("Selection Error : Please select a valid COM port"),Qt::red);
        }
        serial->setPortName(portName);

        // 2. Set UART Configurations (Hardcoded or from UI)
        if (!serial->setBaudRate(921600)) {
            action_log(("invalid baud rate configuration"),Qt::red);
        }
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);

        // 3. Attempt to Open the Port
        if (serial->open(QIODevice::ReadWrite)) {
            appendToFile("--------------------------------------------------------------");
            appendToFile(QString("--------------------Serial Number : %1--------------------").arg(ui->serial_number->text()));
            appendToFile("--------------------------------------------------------------");
            ui->action_log->append("----------------------------------------------------------");
            ui->action_log->append(QString("------------------Serial Number : %1------------------").arg(ui->serial_number->text()));
            ui->action_log->append("----------------------------------------------------------");
            action_log("COM PORT OPENED SUCCESSFULLY",Qt::green);
            uint16_t board_id =  uart_read(0x00);
            if(board_id == 0x8302){
                ui->connect->setText("DISC&ONNECT");
                ui->ref->setEnabled(false);
                ui->com_port->setEnabled(false);
                ui_enable(true);
            }
            else{
                action_log("Failed to read board ID",Qt::red);
                serial->close();
                ui_enable(false);
            }

        } else {
            action_log("Failed to open COM Port",Qt::red);
            ui_enable(false);
        }

    }
    else{
        // If the port is already open, the button should "Disconnect"
        if (serial->isOpen()) {
            serial->close();
            ui->connect->setText("C&ONNECT");
            action_log("COM PORT CLOSED SUCCESSFULLY",Qt::green);
            ui->action_log->setTextColor(Qt::black);
            ui->action_log->append("----------------------------------------------------------");
            appendToFile("--------------------------------------------------------------");
            ui->ref->setEnabled(true);
            ui->com_port->setEnabled(true);
            ui_enable(false);
        }
    }
    ui->connect->setEnabled(true);
}

void auto_mode::on_read_clicked()
{
    ui->read->setEnabled(false);
    if (!serial->isOpen()) {
        action_log("Please connect to COM Port first",Qt::red);
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
    }
    else {
        if (serial->waitForReadyRead(50)) {
                QByteArray data = serial->readAll();
//                data = qToBigEndian(data);
                data = data.mid(1,2);
                quint16 dataa = 0;
                dataa = (static_cast<quint8>(data[2]) << 8) | (static_cast<quint8>(data[1]));
                ui->data_edt->setText(data.toHex(' ').toUpper());
                frame.clear();
                action_log("Debug read success",Qt::green);
            } else {
                action_log("Debug read Failed",Qt::red);
                    ui->action_log->setTextColor(Qt::red);

            }
        serial->flush();
    }
    ui->read->setEnabled(true);
}

void auto_mode::on_write_clicked()
{
    ui->write->setEnabled(false);
    if (!serial->isOpen()) {
        action_log("Please connect to COM Port first",Qt::red);
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
    serial->write(frame);
    serial->waitForBytesWritten(10);
    serial->waitForReadyRead(50);
    status = serial->readAll();
    if(static_cast<quint8>(status[0]) == 0x50){
        action_log("Debug Write Success",Qt::green);
    }
    else{
        action_log("Debug Write Failed",Qt::red);
    }
    ui->write->setEnabled(true);
}

bool auto_mode::uart_write(uint16_t address, uint16_t data, bool isWrite)
{
    if (!serial->isOpen()) return false;

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
    qint64 bytesWritten = serial->write(frame);
    serial->waitForBytesWritten(10);
    if (bytesWritten == -1) {
        action_log("Failed to write data: " + serial->errorString(),Qt::red);
        return false;
    }
    else {
        if (serial->waitForReadyRead(50)) {
                status = serial->readAll();
                if(static_cast<quint8>(status[0]) == 0x50){
//                    action_log("uart write success",Qt::green);
                    return true;
                }
                else{
                    action_log("uart write failed",Qt::red);
                    return false;
                }
            }
        else{
            return false;
        }
    }
    serial->flush();
}

quint16 auto_mode::uart_read(uint16_t address)
{
    uint16_t data = 0;
    if (!serial->isOpen()) {
        action_log("Please connect to COM Port first",Qt::red);
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

        // 4. Send Request
        qint64 bytesWritten = serial->write(frame);
        if (!serial->waitForBytesWritten(10) || bytesWritten == -1) {
            action_log("Failed to write UART data: " + serial->errorString(), Qt::red);
        }
        // 5. Wait for Response (Expecting 257 bytes)
        // We wait until the buffer has the minimum required bytes
        const int expectedSize = 4;
        while (serial->bytesAvailable() < expectedSize) {
            if (!serial->waitForReadyRead(50)) { // 2 second timeout
    //            action_log("UART Timeout: Received only " + QString::number(serial->bytesAvailable()) + " bytes", Qt::red);
                break;
            }
        }
        // 6. Read and Clean Data
        QByteArray dataa;
        if (serial->bytesAvailable() >= expectedSize) {
            dataa = serial->read(expectedSize);
            uint8_t checksum = 0;
            for(int i = 0; i < 3; ++i) {
                checksum ^= static_cast<uint8_t>(dataa.at(i));
            }
            if(checksum == static_cast<quint8>(dataa[3]))
            {
                data = (static_cast<quint8>(dataa[1]) << 8) | static_cast<quint8>(dataa[2]);
                serial->flush();
            }
            else{
                action_log(QString("Failed to read address 0x%1 , checksum error").arg(address,0,16),Qt::red);
            }
        } else {
            dataa = serial->readAll(); // Grab whatever partial data is there
        }


    }
    return data;
}



//QByteArray auto_mode::uart_bulk_read(uint16_t address, uint16_t count)
//{
//    QByteArray dataa;
//    uint16_t data = 0;
//    if (!serial->isOpen()) {
//        action_log("Please connect to COM Port first",Qt::red);
//    }
//    else
//    {
//        // 3. Convert Hex String to QByteArray
//        QByteArray frame;
//        // Byte 0: Header
//        m_address1 = address;
//        frame.append(static_cast<char>(bulk_header1));
//        uint16_t addrBlock = 0x7FFF & m_address1;
//        frame.append(static_cast<char>((addrBlock >> 8) & 0xFF)); // High Byte
//        frame.append(static_cast<char>(addrBlock & 0xFF));        // Low Byte
//        frame.append(static_cast<char>(count >> 8));
//        frame.append(static_cast<char>(count & 0xFF));


//        // Byte 3 & 4: Data (16 bit)
//        //frame.append(static_cast<char>((m_data1 >> 8) & 0xFF));    // MSB
//        //frame.append(static_cast<char>(m_data1 & 0xFF));           // LSB

//        // Byte 5: XOR Checksum (Calculated over all 5 previous bytes)
//        m_checksum1 = 0;
//        for(int i = 0; i < frame.size(); ++i) {
//            m_checksum1 ^= static_cast<uint8_t>(frame.at(i));
//        }
//        frame.append(static_cast<char>(m_checksum1));

//        // 4. Send over Serial
//        qint64 bytesWritten = serial->write(frame);
//        serial->waitForBytesWritten(10);
//        if (bytesWritten == -1) {
//            action_log("Failed to write UART data" + serial->errorString(),Qt::red);
//        }
//        else
//        {

//            while (serial->bytesAvailable() < 257)
//            {
//                if (serial->bytesAvailable() >= 257)
//                {
//                    dataa = serial->read(257); // Read exactly what you expect
//                }
//                else {
//                    action_log("Timeout: Received only " + QString::number(serial->bytesAvailable()) + " bytes", Qt::red);
//                }

//                // Safety check before removing header/footer
//                if (dataa.size() >= 2) {
//                    dataa.remove(0, 1);               // Remove Header
//                    dataa.remove(dataa.size() - 1, 1); // Remove Checksum
//                } else {
//                    dataa.clear(); // Ensure we don't return partial "garbage" data
//                }

//                return dataa;
//            }
//            dataa = serial->readAll();
//        }
//    }
//    dataa.remove(0,1);
//    dataa.remove(dataa.size()-1,1);
//    return dataa;
//}

QByteArray auto_mode::uart_bulk_read(uint16_t address, uint16_t count)
{
    QByteArray dataa;

    // 1. Connection Check
    if (!serial || !serial->isOpen()) {
        action_log("Please connect to COM Port first", Qt::red);
        return dataa; // Return empty array
    }

    // 2. Build the Request Frame
    QByteArray frame;
    frame.append(static_cast<char>(bulk_header1)); // Byte 0: Header

    // Address (16-bit) - Masking and Splitting into High/Low bytes
    uint16_t addrBlock = 0x7FFF & address;
    frame.append(static_cast<char>((addrBlock >> 8) & 0xFF)); // High Byte
    frame.append(static_cast<char>(addrBlock & 0xFF));        // Low Byte

    // Count (16-bit)
    frame.append(static_cast<char>((count >> 8) & 0xFF));     // High Byte
    frame.append(static_cast<char>(count & 0xFF));            // Low Byte

    // 3. Calculate Checksum (XOR over all previous bytes)
    uint8_t checksum = 0;
    for(int i = 0; i < frame.size(); ++i) {
        checksum ^= static_cast<uint8_t>(frame.at(i));
    }
    frame.append(static_cast<char>(checksum));

    // 4. Send Request
    qint64 bytesWritten = serial->write(frame);
    if (!serial->waitForBytesWritten(100) || bytesWritten == -1) {
        action_log("Failed to write UART data: " + serial->errorString(), Qt::red);
        return dataa;
    }

    // 5. Wait for Response (Expecting 257 bytes)
    // We wait until the buffer has the minimum required bytes
    const int expectedSize = count+2;
    while (serial->bytesAvailable() < expectedSize) {
        if (!serial->waitForReadyRead(2000)) { // 2 second timeout
//            action_log("UART Timeout: Received only " + QString::number(serial->bytesAvailable()) + " bytes", Qt::red);
            break;
        }
    }
    // 6. Read and Clean Data
    if (serial->bytesAvailable() >= expectedSize) {
        dataa = serial->read(expectedSize); // Read exactly 257 bytes
    } else {
        dataa = serial->readAll(); // Grab whatever partial data is there
    }

    // CRITICAL: Safety check before removing first/last bytes to prevent Segfault
//    if (dataa.size() >= 2) {
//        dataa.remove(0, 1);               // Remove Header (Index 0)
//        dataa.remove(dataa.size() - 1, 1); // Remove Checksum (Last Index)
//    } else if (dataa.size() > 0) {
//        // If we got only 1 byte, we can't remove both header and footer
//        action_log("Error: Packet too small to be valid.", Qt::red);
//        dataa.clear();
//    }
    dataa.remove(0,1);
    dataa.remove(dataa.size()-1,1);
    return dataa;
}

void auto_mode::uart_bulk_write(uint16_t address, uint16_t count, QByteArray data)
{
    QByteArray dataa;
    dataa = data;

    // 2. Build the Request Frame
    QByteArray frame;
    frame.append(static_cast<char>(bulk_header1)); // Byte 0: Header

    uint16_t addrBlock = (true ? 0x8000 : 0x0000) | (address & 0x7FFF);
    frame.append(static_cast<char>((addrBlock >> 8) & 0xFF)); // High Byte
    frame.append(static_cast<char>(addrBlock & 0xFF));        // Low Byte

    // Count (16-bit)
    frame.append(static_cast<char>((count >> 8) & 0xFF));     // High Byte
    frame.append(static_cast<char>(count & 0xFF));            // Low Byte
    frame.append(data);

    // 3. Calculate Checksum (XOR over all previous bytes)
    uint8_t checksum = 0;
    for(int i = 0; i < frame.size(); ++i) {
        checksum ^= static_cast<uint8_t>(frame.at(i));
    }
    frame.append(static_cast<char>(checksum));

    // 4. Send Request
    qint64 bytesWritten = serial->write(frame);
    if (!serial->waitForBytesWritten(2000) || bytesWritten == -1) {
        action_log("Failed to write UART data: " + serial->errorString(), Qt::red);
    }
    serial->waitForReadyRead(50);
    status = serial->readAll();
    if(static_cast<quint8>(status[0]) == 0x50){
//        action_log("Debug Write Success",Qt::green);
    }
    else{
        action_log(QString("Debug Write Failed :%1").arg(status[0],16),Qt::red);
    }
}




bool auto_mode::on_freq_cnfg_clicked()
{
        QString freqStr = ui->freq_ip->text().trimmed();
        // 2. Convert Hex String to Unsigned Int (Base 16)
        bool ok;
        bool sts = true;
        req_freq = freqStr.toUInt(&ok);
        pul_prd_time = ((1.0/req_freq)/(1.0/smp_freq))-1;
        valid_pul_time = (1.0/req_freq)/(1.0/smp_freq);
        on_time = ((1.0/req_freq)/(1.0/smp_freq))/2.0;
        bool write_sts = uart_write(on_pul_reg_addr,on_time,true);
        serial->waitForBytesWritten(10);
        serial->waitForReadyRead(50);
        if(write_sts == true){
//          action_log("ON Pulse configiration success",Qt::green);
        }
        else{
            action_log("ON Pulse configiration failed",Qt::red);
            sts = false;
        }
        write_sts = uart_write(period_reg_lsb_addr,pul_prd_time,true);
        serial->waitForBytesWritten(10);
        serial->waitForReadyRead(50);
        status = serial->readAll();
        if(write_sts == true){
//          action_log("Pulse period configuration success",Qt::green);
        }
        else{
            action_log("Pulse period configiration failed",Qt::red);
            sts = false;
        }
        write_sts = uart_write(pul_valid_addr,pul_prd_time,true);
        serial->waitForBytesWritten(10);
        serial->waitForReadyRead(50);
        status = serial->readAll();
        if(write_sts == true){
//          action_log("valid Pulse period configiration success",Qt::green);
        }
        else{
            action_log("valid Pulse period configiration failed",Qt::red);
            sts = false;
        }
        if(sts == true){
            action_log("Frequency Configuration success",Qt::green);
        }
        else{
            action_log("Frequency Configuration failed",Qt::red);
        }
        return sts;

}

void auto_mode::on_tx_enable_clicked()
{
    ui->tx_enable->setEnabled(false);
    bool sts = false;
    if(ui->tx_enable->text()== "&ENABLE"){
        sts = uart_write(pul_enable_addr,0x01,true);
        serial->waitForBytesWritten(10);
        serial->waitForReadyRead(50);
        if(sts == true){
            action_log("TX enable configuration success",Qt::green);
        }
        else{
            action_log("TX enable configuration failed",Qt::red);
        }
        ui->tx_enable->setText("DISABL&E");
    }
    else{
        sts = uart_write(pul_enable_addr,0x00,true);
        serial->waitForBytesWritten(10);
        serial->waitForReadyRead(50);
        if(sts == true){
            action_log("TX disable configuration success",Qt::green);
        }
        else{
            action_log("TX disable configuration failed",Qt::red);
        }
        ui->tx_enable->setText("&ENABLE");
    }
    ui->tx_enable->setEnabled(true);
}


void auto_mode::on_tx_config_clicked()
{
    auto_mode::on_cn_tx_cnfg_clicked();
    auto_mode::on_rs422_tx_cnfg_clicked();
    auto_mode::on_lvttl_tx_cnfg_clicked();
    auto_mode::on_freq_cnfg_clicked();
}

void auto_mode::action_log(QString message, Qt::GlobalColor color)
{
    QString time_stamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh.mm.ss.zzz");
    QString log_line = QString("[%1] %2").arg(time_stamp).arg(message);
    ui->action_log->setTextColor(color);
    ui->action_log->append(log_line);
    appendToFile(log_line);
}


void auto_mode::on_lvds_rx_read_clicked()
{
    // 3. Optional: Set to Flat style for a cleaner look
    ui->lvds_freq_35->setSegmentStyle(QLCDNumber::Flat);
    ui->lvds_rx_read->setEnabled(false);
    ui->Receiver_group->setEnabled(false);
    quint16 freq = 0;
    quint16 glitch_cnt = 0;
    quint16 fail_cnt = 0;
    int counter = 0;

    bool read_sts = true;
    // Iterate through every button in the group
    // The loop now knows what 'lvds_rx_group' is
    for (QAbstractButton *button : rx_lvds_group[0]->buttons()) {
        lvds_freq_group[counter]->display("-----");
        lvds_glitch_group[counter]->display("-----");
        counter++;
    }
    for (QAbstractButton *button : rx_lvds_group[1]->buttons()) {
        lvds_freq_group[counter]->display("-----");
        lvds_glitch_group[counter]->display("-----");
        counter++;
    }
    counter = 0;
    for (QAbstractButton *button : rx_lvds_group[0]->buttons()) {
        if (button->isChecked()) {
            freq = uart_read(lvds_grp_pul_cnt_addr_lst[lvds_rx_index[counter]]);
            glitch_cnt = uart_read(lvds_grp_glitch_cnt_addr_lst[lvds_rx_index[counter]]);
            lvds_freq_group[counter]->setDigitCount(5);
            lvds_glitch_group[counter]->setDigitCount(5);
            fail_cnt = (freq >> 1);
            if(freq == 0x01 & glitch_cnt == 0){
                if(cont_mode == true){
                    lvds_freq_group[counter]->display(gpi_lvds_pul_sts_cnt_addr_lst[lvds_rx_index[counter]]);
                    lvds_glitch_group[counter]->display(fail_cnt);
                    ui->status1->setText("FAIL COUNT");
                    ui->status2->setText("FAIL COUNT");
                    ui->status3->setText("FAIL COUNT");
                    if(gpi_lvds_pul_sts_cnt_addr_lst[lvds_rx_index[counter]] == 0){
                        lvds_glitch_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                        lvds_glitch_group[counter]->setStyleSheet("background-color: green; color: black;");
                        lvds_freq_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                        lvds_freq_group[counter]->setStyleSheet("background-color: green; color: black;");              
                        QCoreApplication::processEvents();
                    }
                    else{
                        lvds_glitch_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                        lvds_glitch_group[counter]->setStyleSheet("background-color: red; color: black;");
                        lvds_freq_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                        lvds_freq_group[counter]->setStyleSheet("background-color: red; color: black;");
                        QCoreApplication::processEvents();
                    }

                }
                else
                {
                    ui->status1->setText("PASS / FAIL");
                    ui->status2->setText("PASS / FAIL");
                    ui->status3->setText("PASS / FAIL");
                    lvds_freq_group[counter]->display("PASS");
                    lvds_glitch_group[counter]->display(fail_cnt);
                    lvds_glitch_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                    lvds_glitch_group[counter]->setStyleSheet("background-color: green; color: black;");
                    lvds_freq_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                    lvds_freq_group[counter]->setStyleSheet("background-color: green; color: black;");
                    QCoreApplication::processEvents();
                }

            }
            else if(freq == 0xB055){
                action_log(QString("Failed to read LVDS RX Channel : %1").arg(counter),Qt::red);
                read_sts = false;
            }
            else
            {
                if(cont_mode == true)
                {
                    ui->status1->setText("FAIL COUNT");
                    ui->status2->setText("FAIL COUNT");
                    ui->status3->setText("FAIL COUNT");
                    gpi_lvds_pul_sts_cnt_addr_lst[lvds_rx_index[counter]]++;
                    lvds_freq_group[counter]->display(gpi_lvds_pul_sts_cnt_addr_lst[lvds_rx_index[counter]]);
                    lvds_glitch_group[counter]->display(fail_cnt);
                }
                else
                {
                    ui->status1->setText("PASS / FAIL");
                    ui->status2->setText("PASS / FAIL");
                    ui->status3->setText("PASS / FAIL");
                    lvds_freq_group[counter]->display("FA1L");
                    lvds_glitch_group[counter]->display(fail_cnt);
                    QCoreApplication::processEvents();
                }
                lvds_glitch_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                lvds_glitch_group[counter]->setStyleSheet("background-color: red; color: black;");
                lvds_freq_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                lvds_freq_group[counter]->setStyleSheet("background-color: red; color: black;");
                QCoreApplication::processEvents();
            }
        }
        else{
            lvds_glitch_group[counter]->setStyleSheet("");
            lvds_freq_group[counter]->setStyleSheet("");
            lvds_freq_group[counter]->setDigitCount(5);
            lvds_glitch_group[counter]->setDigitCount(5);
            lvds_freq_group[counter]->display("-----");
            lvds_glitch_group[counter]->display("-----");
            QCoreApplication::processEvents();

        }
        counter++;
    }

    for (QAbstractButton *button : rx_lvds_group[1]->buttons()) {
        if (button->isChecked()) {
            freq = uart_read(lvds_grp_pul_cnt_addr_lst[lvds_rx_index[counter]]);
            glitch_cnt = uart_read(lvds_grp_glitch_cnt_addr_lst[counter]);
            fail_cnt = (freq >> 1);
            QCoreApplication::processEvents();
            if(freq == 0x01 & glitch_cnt == 0){
                if(cont_mode == true){
                    ui->status1->setText("FAIL COUNT");
                    ui->status2->setText("FAIL COUNT");
                    ui->status3->setText("FAIL COUNT");
                    gpi_lvds_pul_sts_cnt_addr_lst[lvds_rx_index[counter]]++;
                    lvds_freq_group[counter]->display(gpi_lvds_pul_sts_cnt_addr_lst[lvds_rx_index[counter]]);
                    lvds_glitch_group[counter]->display(fail_cnt);
                }
                else
                {
                    ui->status1->setText("FAIL COUNT");
                    ui->status2->setText("FAIL COUNT");
                    ui->status3->setText("FAIL COUNT");
                    lvds_freq_group[counter]->display("PASS");
                    lvds_glitch_group[counter]->display(fail_cnt);
                }
                lvds_glitch_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                lvds_glitch_group[counter]->setStyleSheet("background-color: green; color: black;");
                lvds_freq_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                lvds_freq_group[counter]->setStyleSheet("background-color: green; color: black;");
                QCoreApplication::processEvents();
            }
            else if(freq == 0xB055){
                action_log(QString("Failed to read LVDS RX Channel : %1").arg(counter),Qt::red);
                read_sts = false;
            }
            else
            {
                if(cont_mode == true)
                {
                    ui->status1->setText("FAIL COUNT");
                    ui->status2->setText("FAIL COUNT");
                    ui->status3->setText("FAIL COUNT");
                    gpi_lvds_pul_sts_cnt_addr_lst[lvds_rx_index[counter]]++;
                    lvds_freq_group[counter]->display(gpi_lvds_pul_sts_cnt_addr_lst[lvds_rx_index[counter]]);
                    lvds_glitch_group[counter]->display(fail_cnt);
                }
                else
                {
                    ui->status1->setText("FAIL COUNT");
                    ui->status2->setText("FAIL COUNT");
                    ui->status3->setText("FAIL COUNT");
                    lvds_freq_group[counter]->display("FA1L");
                    lvds_glitch_group[counter]->display(fail_cnt);
                    QCoreApplication::processEvents();
                }
                lvds_glitch_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                lvds_glitch_group[counter]->setStyleSheet("background-color: red; color: black;");
                lvds_freq_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                lvds_freq_group[counter]->setStyleSheet("background-color: red; color: black;");
                QCoreApplication::processEvents();
            }
        }
        else{
            lvds_glitch_group[counter]->setStyleSheet("");
            lvds_freq_group[counter]->setStyleSheet("");
            lvds_freq_group[counter]->setDigitCount(5);
            lvds_glitch_group[counter]->setDigitCount(5);
            lvds_freq_group[counter]->display("-----");
            lvds_glitch_group[counter]->display("-----");
            QCoreApplication::processEvents();
        }
        counter++;
    }
    if(read_sts == true){
        action_log("LVDS RX Read success",Qt::green);
    }
    else{
        action_log("LVDS RX Read failed",Qt::red);
    }
    ui->Receiver_group->setEnabled(true);
    ui->lvds_rx_read->setEnabled(true);
}


void auto_mode::on_lvttl_rx_read_clicked()
{
    ui->lvttl_rx_read->setEnabled(false);
    ui->Receiver_group->setEnabled(false);
    quint16 freq = 0;
    quint16 glitch_cnt = 0;
    quint16 fail_cnt = 0;
    int counter = 0;
    bool read_sts = true;
    for (QAbstractButton *button : lvttl_rx_group->buttons()) {
        lvttl_freq_group[counter]->display("-----");
        lvttl_glitch_group[counter]->display("-----");
        counter++;
    }
    counter = 0;
    // Iterate through every button in the group
    // The loop now knows what 'lvds_rx_group' is
    for (QAbstractButton *button : lvttl_rx_group->buttons()) {
        if (button->isChecked()) {
            freq = uart_read(lvttl_grp_pul_cnt_addr_lst[counter]);
            glitch_cnt = uart_read(lvttl_grp_glitch_cnt_addr_lst[counter]);
            fail_cnt = freq >> 1;
            if(freq == 0x01 & glitch_cnt == 0){
                if(cont_mode == true){
                    ui->status5->setText("FAIL COUNT");
                    lvttl_freq_group[counter]->display(gpi_lvttl_pul_sts_cnt_addr_lst[counter]);
                    lvttl_glitch_group[counter]->display(fail_cnt);
                    if(gpi_lvttl_pul_sts_cnt_addr_lst[counter] == 0){
                        lvttl_glitch_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                        lvttl_glitch_group[counter]->setStyleSheet("background-color: green; color: black;");
                        lvttl_freq_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                        lvttl_freq_group[counter]->setStyleSheet("background-color: green; color: black;");
                    }
                    else
                    {
                        lvttl_glitch_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                        lvttl_glitch_group[counter]->setStyleSheet("background-color: red; color: black;");
                        lvttl_freq_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                        lvttl_freq_group[counter]->setStyleSheet("background-color: red; color: black;");
                    }
                }
                else
                {
                    ui->status5->setText("PASS / FAIL");
                    lvttl_freq_group[counter]->display("PASS");
                    lvttl_glitch_group[counter]->display(fail_cnt);
                    lvttl_glitch_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                    lvttl_glitch_group[counter]->setStyleSheet("background-color: green; color: black;");
                    lvttl_freq_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                    lvttl_freq_group[counter]->setStyleSheet("background-color: green; color: black;");
                }
                QCoreApplication::processEvents();
            }
            else if(freq == 0x00)
            {
                if(cont_mode == true){
                    gpi_lvttl_pul_sts_cnt_addr_lst[counter]++;
                    ui->status5->setText("FAIL COUNT");
                    lvttl_freq_group[counter]->display(gpi_lvttl_pul_sts_cnt_addr_lst[counter]);
                    lvttl_glitch_group[counter]->display(fail_cnt);  
                }
                else{
                    ui->status5->setText("PASS / FAIL");
                    lvttl_freq_group[counter]->display("FA1L");
                    lvttl_glitch_group[counter]->display(fail_cnt);
                }

                lvttl_glitch_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                lvttl_glitch_group[counter]->setStyleSheet("background-color: red; color: black;");
                lvttl_freq_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                lvttl_freq_group[counter]->setStyleSheet("background-color: red; color: black;");
                QCoreApplication::processEvents();
            }
            else if(freq != 0xB055){
                ui->action_log->setTextColor(Qt::red);
                ui->action_log->setText("LVDS RX UART Read Failed");
                read_sts = false;
                QCoreApplication::processEvents();
            }
        }
        else{
            lvttl_freq_group[counter]->setStyleSheet("");
            lvttl_glitch_group[counter]->setStyleSheet("");
            lvttl_freq_group[counter]->setDigitCount(5);
            lvttl_glitch_group[counter]->setDigitCount(5);
            lvttl_freq_group[counter]->display("-----");
            lvttl_glitch_group[counter]->display("-----");
            QCoreApplication::processEvents();
        }
        counter++;
    }
    if(read_sts == true){
        action_log("LVTTL RX Read Success",Qt::green);
    }
    else{
        action_log("LVTTL RX Read failed",Qt::red);
    }
    ui->Receiver_group->setEnabled(true);
    ui->lvttl_rx_read->setEnabled(true);
}


void auto_mode::on_RS422_rx_read_clicked()
{
    ui->RS422_rx_read->setEnabled(false);
    ui->Receiver_group->setEnabled(false);
    quint16 freq = 0;
    quint16 glitch_cnt = 0;
    quint16 fail_cnt = 0;
    int counter = 0;
    bool read_sts = true;
    for (QAbstractButton *button : rs422_rx_group->buttons()) {
        rs422_freq_group[counter]->display("-----");
        rs422_glitch_group[counter]->display("-----");
        counter++;
    }
    counter = 0;
    // Iterate through every button in the group
    // The loop now knows what 'lvds_rx_group' is
    for (QAbstractButton *button : rs422_rx_group->buttons()) {
        if (button->isChecked()) {
            freq = uart_read(rs422_grp_pul_cnt_addr_lst[counter]);
            glitch_cnt = uart_read(rs422_grp_glitch_cnt_addr_lst[counter]);
            fail_cnt = freq >> 1;
            rs422_freq_group[counter]->setDigitCount(5);
            rs422_glitch_group[counter]->setDigitCount(5);
            rs422_freq_group[counter]->display("-----");
            rs422_glitch_group[counter]->display("-----");
            QCoreApplication::processEvents();
            if(freq == 0x01 & glitch_cnt== 0){
                if(cont_mode == true){
                    ui->status4->setText("FAIL COUNT");
                    rs422_freq_group[counter]->display(gpi_rs422_pul_sts_cnt_addr_lst[counter]);
                    rs422_glitch_group[counter]->display(fail_cnt);
                    if(gpi_rs422_pul_sts_cnt_addr_lst[counter] == 0){
                        rs422_glitch_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                        rs422_glitch_group[counter]->setStyleSheet("background-color: green; color: black;");
                        rs422_freq_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                        rs422_freq_group[counter]->setStyleSheet("background-color: green; color: black;");
                    }
                    else
                    {
                        rs422_glitch_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                        rs422_glitch_group[counter]->setStyleSheet("background-color: red; color: black;");
                        rs422_freq_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                        rs422_freq_group[counter]->setStyleSheet("background-color: red; color: black;");
                    }
                }
                else
                {
                    ui->status4->setText("PASS / FAIL");
                    rs422_freq_group[counter]->display("PASS");
                    rs422_glitch_group[counter]->display(fail_cnt);
                    rs422_glitch_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                    rs422_glitch_group[counter]->setStyleSheet("background-color: green; color: black;");
                    rs422_freq_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                    rs422_freq_group[counter]->setStyleSheet("background-color: green; color: black;");
                }
            }
            else if(freq == 0x000)
            {
                if(cont_mode == true){
                    ui->status4->setText("FAIL COUNT");
                    gpi_rs422_pul_sts_cnt_addr_lst[counter]++;
                    rs422_freq_group[counter]->display(gpi_rs422_pul_sts_cnt_addr_lst[counter]);
                    rs422_glitch_group[counter]->display(fail_cnt);
                }
                else{
                    ui->status4->setText("PASS / FAIL");
                    rs422_freq_group[counter]->display("FA1L");
                    rs422_glitch_group[counter]->display(fail_cnt);
                }

                rs422_glitch_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                rs422_glitch_group[counter]->setStyleSheet("background-color: red; color: black;");
                rs422_freq_group[counter]->setSegmentStyle(QLCDNumber::Flat);
                rs422_freq_group[counter]->setStyleSheet("background-color: red; color: black;");
            }
            else if(freq != 0xB055){
                ui->action_log->setTextColor(Qt::red);
                ui->action_log->setText("LVDS RX UART Read Failed");
                QCoreApplication::processEvents();
                read_sts = false;
            }
            }
        else
        {
            rs422_freq_group[counter]->setStyleSheet("");
            rs422_glitch_group[counter]->setStyleSheet("");
            rs422_freq_group[counter]->setDigitCount(5);
            rs422_glitch_group[counter]->setDigitCount(5);
            rs422_freq_group[counter]->display("-----");
            rs422_glitch_group[counter]->display("-----");
            QCoreApplication::processEvents();
        }
        counter++;
    }
    if(read_sts == true){
        action_log("RS422 RX Read Success",Qt::green);
    }
    else{
        action_log("RS422 RX Read failed",Qt::red);
    }
    ui->Receiver_group->setEnabled(true);
    ui->RS422_rx_read->setEnabled(true);
}


void auto_mode::on_cn_rx1_clicked(bool checked)
{
    if(checked){

        ui->lvds_tx_en_1 ->setEnabled(true);
        ui->lvds_tx_en_5 ->setEnabled(true);
        ui->lvds_tx_en_6 ->setEnabled(true);
        ui->lvds_tx_en_9 ->setEnabled(true);
        ui->lvds_tx_en_18 ->setEnabled(true);
        ui->lvds_tx_en_19 ->setEnabled(true);
        ui->lvds_tx_en_21 ->setEnabled(true);
        ui->lvds_tx_en_22 ->setEnabled(true);
        ui->lvds_tx_en_26 ->setEnabled(true);
        ui->lvds_tx_en_29 ->setEnabled(true);
        ui->lvds_rx_en->setChecked(true);
        for (QAbstractButton *button : lvds_cn_rx1_group->buttons()) {
            button->setChecked(true);
            button->setEnabled(true);
        }
    }
    else{
        ui->lvds_rx_en->setChecked(false);
        for (QAbstractButton *button : lvds_cn_rx1_group->buttons()) {
            button->setChecked(false);
            button->setEnabled(true);
        }

    }
    for (QAbstractButton *button : lvds_cn_rx2_group->buttons()) {
        button->setChecked(false);
        button->setEnabled(false);
    }
    for (QAbstractButton *button : lvds_cn_rx2_tx_group->buttons()) {
        button->setChecked(checked);
    }
    for (QAbstractButton *button : lvds_cn_rx2_tx_dis_group->buttons()) {
        button->setChecked(checked);
    }
    if(!checked){
        for (QAbstractButton *button : lvds_cn_rx2_group->buttons()) {
            button->setEnabled(true);
        }
    }
    ui->cn_rx2->setChecked(false);
    ui->lvds_tx_en->setChecked(checked);
    ui->rx->setChecked(false);
}

void auto_mode::on_cn_rx2_clicked(bool checked)
{
    if(checked){

        ui->lvds_tx_en_1 ->setEnabled(false);
        ui->lvds_tx_en_5 ->setEnabled(false);
        ui->lvds_tx_en_6 ->setEnabled(false);
        ui->lvds_tx_en_9 ->setEnabled(false);
        ui->lvds_tx_en_18 ->setEnabled(false);
        ui->lvds_tx_en_19 ->setEnabled(false);
        ui->lvds_tx_en_21 ->setEnabled(false);
        ui->lvds_tx_en_22 ->setEnabled(false);
        ui->lvds_tx_en_26 ->setEnabled(false);
        ui->lvds_tx_en_29 ->setEnabled(false);
        for (QAbstractButton *button : lvds_cn_rx1_group->buttons())
        {
            button->setEnabled(false);
        }
        for (QAbstractButton *button : lvds_cn_rx2_group->buttons())
        {
            button->setEnabled(true);
        }
        ui->lvds_rx_en->setChecked(true);

    }
    else
    {
        ui->lvds_tx_en_1 ->setEnabled(true);
        ui->lvds_tx_en_5 ->setEnabled(true);
        ui->lvds_tx_en_6 ->setEnabled(true);
        ui->lvds_tx_en_9 ->setEnabled(true);
        ui->lvds_tx_en_18 ->setEnabled(true);
        ui->lvds_tx_en_19 ->setEnabled(true);
        ui->lvds_tx_en_21 ->setEnabled(true);
        ui->lvds_tx_en_22 ->setEnabled(true);
        ui->lvds_tx_en_26 ->setEnabled(true);
        ui->lvds_tx_en_29 ->setEnabled(true);
        for (QAbstractButton *button : lvds_cn_rx1_group->buttons())
        {
            button->setEnabled(true);
        }
        ui->lvds_rx_en->setChecked(false);
    }
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
    ui->lvds_tx_en->setChecked(checked);
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
    ui->rs422_tx_en_34->setText("CHANNEL 57");
    ui->rs422_rx2->setChecked(false);
    ui->rs422_rx3->setChecked(false);
    ui->rs422_rx4->setChecked(false);
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
    ui->rs422_tx_en_34->setText("CHANNEL 58");
    ui->rs422_rx3->setChecked(false);
    ui->rs422_rx4->setChecked(false);
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
    ui->rs422_tx_en_34->setText("CHANNEL 59");
    ui->rs422_rx4->setChecked(false);
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
    ui->rs422_tx_en_34->setText("CHANNEL 60");
    ui->rx->setChecked(false);
}

void auto_mode::ui_enable(bool enable)
{
    ui->Transmitter_group->setEnabled(enable);
    ui->Receiver_group->setEnabled(enable);
    ui->Debug_group->setEnabled(enable);
}


void auto_mode::on_read_all_clicked()
{
    ui->read_all->setChecked(false);
    on_lvds_rx_read_clicked();
    on_RS422_rx_read_clicked();
    on_lvttl_rx_read_clicked();
    ui->read_all->setChecked(true);
}

void auto_mode::on_cyc_start_clicked()
{
    if(ui->cyc_start->text()=="START"){
        ui->cyc_start->setText("STOP");
    }
    else{
        ui->cyc_start->setText("START");
        ui->cyc_start->setEnabled(false);
        QCoreApplication::processEvents();
    }

    ui->cyc_cnt->setEnabled(false);
    cont_mode = true;
    QString count = ui->cyc_cnt->text().trimmed();
    // 2. Convert Hex String to Unsigned Int (Base 16)
    bool ok;
    int cnt = count.toUInt(&ok);
    ui->cyc_prog->setRange(1,cnt);
    for (int i = 1; i <= cnt ; i++){
        if(ui->cyc_start->text()=="START"){
            ui->cyc_start->setEnabled(true);
            break; 
        }
        on_read_all_clicked();
        QThread::sleep((ui->delay->text().trimmed()).toUInt(&ok));
        ui->cyc_prog->setValue(i);
        ui->present_cnt->setText(QString::number(i));
        QCoreApplication::processEvents();
    }
    ui->cyc_cnt->setEnabled(true);
    ui->cyc_start->setEnabled(true);
    cont_mode = false;
    ui->cyc_start->setText("START");
    gpi_lvds_pul_sts_cnt_addr_lst.assign(56,0x0000);
    gpi_rs422_pul_sts_cnt_addr_lst.assign(4,0x0000);
    gpi_lvttl_pul_sts_cnt_addr_lst.assign(8,0x0000);
}


void auto_mode::flash_chip_erase()
{
    ui->flash_chip_erase->setEnabled(false);
    bool sts = true;
    uint16_t data;
    sts = uart_write(flash_inst_reg, 0x6, true);
    sts = uart_write(flash_inst_reg, 0x60, true);
    while (flash_erase_sts == false){
        sts = uart_write(flash_inst_reg, 0x5, true);
        data = uart_read(flash_read_data_reg);
        if (data == 0x03){
//            action_log(QString("flash_read_data_reg : %1").arg(data),Qt::red);
            flash_erase_sts = false;
        }
        else{
            flash_erase_sts = true;
        }
        QCoreApplication::processEvents();
    }
    ui->action_log->setTextColor(Qt::green);
    action_log("Flash chip erase success",Qt::green);
    ui->flash_chip_erase->setEnabled(true);
}

uint8_t auto_mode::flash_single_read(uint32_t address,uint16_t data)
{

    uint16_t flash_address_lsb ;
    uint16_t flash_address_msb ;
    flash_address_lsb = (address) & 0xFFFF;
    flash_address_msb = (address >> 16) & 0xFF;
//    action_log(QString("LSB : %1").arg(flash_address_lsb),Qt::green);
//    action_log(QString("MSB : %1").arg(flash_address_msb),Qt::green);
    bool sts = true;
    while (flash_mode == false){
        sts = uart_write(flash_inst_reg, 0x5, true);
        data = uart_read(flash_read_data_reg);
        if (data != 0x0){
            action_log(QString("flash_read_data_reg : %1").arg(data),Qt::red);
            flash_mode = false;
        }
        else if(data == 0x0){
            flash_mode = true;
        }
    }
    sts = uart_write(flash_inst_reg, 0x4, true);
    sts = uart_write(flash_lsb_addr_reg, flash_address_lsb, true);
    sts = uart_write(flash_msb_addr_reg, flash_address_msb, true);
    sts = uart_write(flash_data_cnt_reg, 0x1, true);
    sts = uart_write(flash_inst_reg, 0x3, true);
    data = uart_read(flash_read_data_reg);
    return data;
}


QByteArray auto_mode::flash_bulk_read(uint32_t address,uint16_t count)
{
    bool sts = true;
    QByteArray data;
    uint16_t flash_address_lsb ;
    uint16_t flash_address_msb ;
    flash_address_lsb = (address) & 0xFFFF;
    flash_address_msb = (address >> 16) & 0xFF;
    sts = uart_write(flash_inst_reg, 0x4, true);
    sts = uart_write(flash_data_cnt_reg, count, true);
    sts = uart_write(flash_lsb_addr_reg, flash_address_lsb, true);
    sts = uart_write(flash_msb_addr_reg, flash_address_msb, true);
    sts = uart_write(flash_inst_reg, 0x3, true);
    while (uart_read(flash_read_fifo_cnt_reg) != count ){
        sts = uart_write(flash_inst_reg, 0x3, true);
    }
    data = uart_bulk_read(flash_read_fifo_data_reg,count);
    return data;
}

void auto_mode::flash_full_memory_write(){
    ui->flash_write_prog->setRange(1,0x1f);
    for(int i=0 ; i< 0x1f; i++)
    {
        flash_single_write(i,i);
        ui->flash_write_prog->setValue(i);
//        ui->action_log->setText(QString::number(i,16));
        QCoreApplication::processEvents();
    }
}


void auto_mode::flash_single_write(uint32_t address,uint16_t data)
{
    uint16_t flash_address_lsb ;
    uint16_t flash_address_msb ;
    flash_address_lsb = (address) & 0xFFFF;
    flash_address_msb = (address >> 16) & 0xFF;
//    action_log(QString("LSB : %1").arg(flash_address_lsb,16),Qt::green);
//    action_log(QString("MSB : %1").arg(flash_address_msb,16),Qt::green);
//    action_log(QString("DATA : %1").arg(data,16),Qt::green);
    bool sts = true;
    sts = uart_write(flash_inst_reg, 0x6, true);
    sts = uart_write(flash_data_cnt_reg, 0x1, true);
    sts = uart_write(flash_lsb_addr_reg, flash_address_lsb, true);
    sts = uart_write(flash_msb_addr_reg, flash_address_msb, true);
    sts = uart_write(flash_write_reg ,data, true);
    sts = uart_write(flash_inst_reg, 0x2, true);
    sts = uart_write(flash_inst_reg, 0x5, true);
    while (uart_read(flash_read_data_reg ) != 0x0 ){
        sts = uart_write(flash_inst_reg, 0x2, true);
        sts = uart_write(flash_inst_reg, 0x5, true);
    }
    sts = uart_write(flash_inst_reg, 0x4, true);
}


void auto_mode::flash_bulk_write(uint32_t address,QByteArray data)
{

    uint16_t flash_address_lsb ;
    uint16_t flash_address_msb ;
    flash_address_lsb = (address) & 0xFFFF;
    flash_address_msb = (address >> 16) & 0xFF;
//    action_log(QString("LSB : %1").arg(flash_address_lsb,16),Qt::green);
//    action_log(QString("MSB : %1").arg(flash_address_msb,16),Qt::green);
//    action_log(QString("DATA : %1").arg(data,16),Qt::green);
    bool sts = true;
    sts = uart_write(flash_inst_reg, 0x6, true);
    uart_bulk_write(flash_write_reg,data.size(),data);
    sts = uart_write(flash_data_cnt_reg, data.size(), true);
    sts = uart_write(flash_lsb_addr_reg, flash_address_lsb, true);
    sts = uart_write(flash_msb_addr_reg, flash_address_msb, true);
    sts = uart_write(flash_inst_reg, 0x2, true);
    sts = uart_write(flash_inst_reg, 0x5, true);
    while (uart_read(flash_read_data_reg ) != 0x0 ){
        sts = uart_write(flash_inst_reg, 0x2, true);
        sts = uart_write(flash_inst_reg, 0x5, true);
    }
    sts = uart_write(flash_inst_reg, 0x4, true);
}

void auto_mode::on_flash_read_clicked()
{
//    flash_chip_erase();
    bool ok;
    uint32_t address = ui->flash_address_edt->text().toUInt(&ok, 16);
    uint16_t data = ui->flash_data_edt->text().trimmed().toUInt(&ok, 16);
    uint8_t flash_data = flash_single_read(address,data);
    ui->flash_data_edt->setText(QString(QString::number(flash_data, 16).toUpper()));
    action_log("Flash Read success",Qt::green);
}

QByteArray auto_mode::on_flash_bulk_read_clicked()
{
    QByteArray data;
    bool ok;
    uint32_t address = ui->flash_address_edt->text().toUInt(&ok, 32);
    uint16_t count = ui->flash_count->text().toUInt(&ok, 16);
    data = flash_bulk_read(address,count);
    for(int i=0;i<data.size();i++){
        action_log(QString("Address 0x%1 | Data : 0x%2")
            .arg(address, 2, 16, QChar('0'))
            .arg(data[i], 2, 16, QChar('0')), Qt::green);
        address = address +1 ;
    }
    return data;
}


void auto_mode::on_flash_write_clicked()
{
    bool ok;
    uint32_t address = ui->flash_address_edt->text().toUInt(&ok, 16);
    uint16_t data = ui->flash_data_edt->text().trimmed().toUInt(&ok, 16);
    flash_single_write(address,data);
    action_log("Flash Write success",Qt::green);
}

void auto_mode::on_flash_chip_erase_clicked()
{
    flash_chip_erase();
}

void auto_mode::on_action_log_clr_clicked()
{
    ui->action_log->setText("");
}

void auto_mode::on_flash_full_mem_write_clicked()
{
//    ui->flash_full_mem_write->setEnabled(false);
//    flash_full_memory_write();
//    ui->flash_full_mem_write->setEnabled(true);
    flash_full_mem_write();
}


void auto_mode::on_uart_bulk_read_clicked()
{
    bool ok;
    uint16_t count = ui->uart_count->text().toUInt(&ok, 16);
    uint16_t address = ui->address_edt->text().toUInt(&ok, 16);
    QByteArray data = uart_bulk_read(address,count);
    for(int i=0;i<data.size();i++){
        action_log(QString("Address 0x%1 | Data : 0x%2")
            .arg(address, 2, 16, QChar('0'))
            .arg(data[i], 2, 16, QChar('0')), Qt::green);
        address = address +1 ;

    }
}


void auto_mode::on_uart_bulk_write_clicked()
{
    QByteArray data;
    for (int j=0 ; j<=31; j++){
        for (uint8_t i=0 ;i<= 7; i++){
            data.append(i);
            action_log(QString::number(data.size()),Qt::green);
        }
    }
    uart_bulk_write(0x00,data.size(),data);
}

void auto_mode::on_flash_bulk_write_clicked()
{
    QByteArray data;
    for (int j=0 ; j<=31; j++){
        for (uint8_t i=0 ;i<= 7; i++){
            data.append(i);
            action_log(QString::number(data.size()),Qt::green);
        }
    }
    flash_bulk_write(0x00,data);
}

void auto_mode::flash_full_mem_write()
{
    ui->flash_write_prog->setRange(0,131071);
    flash_chip_erase();
    QByteArray data;
    for (int j=0 ; j<=31; j++){
        for (uint8_t i=0 ;i<= 7; i++){
            data.append(i);
//            action_log(QString::number(data.size()),Qt::green);
        }
    }
    for(int i=0; i<= 131071;i++){
        uint32_t address = i+255;
        flash_bulk_write(address,data);
        ui->flash_write_prog->setValue(i);
        QCoreApplication::processEvents();
    }

}


void auto_mode::on_DEBUG_clicked()
{
    if(debug_visibility == true){
        ui->test_mode->setVisible(false);
        ui->flash_group->setVisible(false);
        ui->flash_group_1->setVisible(false);
        ui->Debug_group->setVisible(false);
        ui->uart_bulk_r->setVisible(false);
        ui->rx->setVisible(false);
        debug_visibility = false;
    }
    else{
        ui->test_mode->setVisible(true);
        ui->flash_group->setVisible(true);
        ui->flash_group_1->setVisible(true);
        ui->Debug_group->setVisible(true);
        ui->uart_bulk_r->setVisible(true);
        ui->rx->setVisible(true);
        debug_visibility = true;
    }
}



