#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <vector>
#include <cstdint>

uint8_t  m_header = 0xA7;   // 8-bit Header
bool     m_isWrite = true;  // 1-bit R/W flag
uint16_t m_address = 0x0000; // 15-bit Address (using 16-bit type)
uint16_t m_data = 0x0000;    // 16-bit Data
uint8_t  m_checksum = 0x00;

uint16_t on_pul_reg_addr = 0x0108;
uint16_t period_reg_lsb_addr = 0x0109;
uint16_t period_reg_msb_addr = 0x010A;
uint16_t pul_valid_addr = 0x010E;
uint16_t pul_enable_addr = 0x010C;

uint16_t gpo_grp1_addr = 0x0105;
uint16_t gpo_grp2_addr = 0x0106;
uint16_t gpo_grp3_addr = 0x0107;

uint16_t gp1_grp1_live_sts_addr = 0x0100;
uint16_t gp1_grp2_live_sts_addr = 0x0101;
uint16_t gp1_grp3_live_sts_addr = 0x0102;
uint16_t gp1_grp4_live_sts_addr = 0x0103;
uint16_t gp1_grp5_live_sts_addr = 0x0104;
uint16_t gp1_grp6_live_sts_addr = 0x0105;

uint16_t smp_freq = 80; // 80 Mhz
uint16_t req_freq = 1; // 80 Mhz
//uint16_t on_time = ((1/req_freq)/(1/smp_freq))/2;
//uint32_t pul_prd_time = (1/req_freq)/(1/smp_freq);
//uint16_t valid_pul_time = (1/req_freq)/(1/smp_freq);


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
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
    // Frame Variables
     // 8-bit XOR Checksum
    //// - update the gui
}

MainWindow::~MainWindow()
{
    //// - To create a file with all the available action log.
    delete ui;
}



void MainWindow::on_connect_clicked()
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
        ui->connect->setText("Disconnect"

);
        ui->com_port->setEnabled(false); // Lock the UI while connected
        QMessageBox::information(this, "Connected", "Successfully connected to " + portName);
    } else {
        QMessageBox::critical(this, "Port Error", "Failed to open port: " + serial->errorString());
    }
}


void MainWindow::on_ref_clicked()
{
    ui->com_port->clear();
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &portInfo : serialPortInfos) {
            ui->com_port->addItem(portInfo.portName());
        }
}


void MainWindow::on_read_clicked()
{
    if (!serial->isOpen()) {
        QMessageBox::warning(this, "Port Closed", "Please connect to a COM port first!");
        return;
    }


    // 3. Convert Hex String to QByteArray
    QByteArray frame;

    // Byte 0: Header
    frame.append(static_cast<char>(m_header));

    // Byte 1 & 2: [R/W bit (1 bit)] + [Address (15 bits)]
    // Pack R/W into the most significant bit of the 16-bit block
    QString addrStr = ui->address_edt->text().trimmed();

        // 2. Convert Hex String to Unsigned Int (Base 16)
        bool ok;
        uint32_t tempAddr = addrStr.toUInt(&ok, 16);

        // 3. Store in the variable if conversion was successful
        if (ok) {
            // Mask to 15 bits as per your frame requirement
            m_address = static_cast<uint16_t>(tempAddr & 0x7FFF);
        } else {
            QMessageBox::warning(this, "Input Error", "Invalid Address! Please enter a Hex value (e.g., 1A2B)");
            return; // Stop the function if address is bad
        }


    uint16_t addrBlock = (m_isWrite ? 0x0000 : 0x0000) | m_address;
    frame.append(static_cast<char>((addrBlock >> 8) & 0xFF)); // High Byte
    frame.append(static_cast<char>(addrBlock & 0xFF));        // Low Byte

    // Byte 3 & 4: Data (16 bit)
    //frame.append(static_cast<char>((m_data >> 8) & 0xFF));    // MSB
    //frame.append(static_cast<char>(m_data & 0xFF));           // LSB

    // Byte 5: XOR Checksum (Calculated over all 5 previous bytes)
    m_checksum = 0;
    for(int i = 0; i < frame.size(); ++i) {
        m_checksum ^= static_cast<uint8_t>(frame.at(i));
    }
    frame.append(static_cast<char>(m_checksum));

    // 4. Send over Serial
    qint64 bytesWritten = serial->write(frame);

    if (bytesWritten == -1) {
        QMessageBox::critical(this, "Error", "Failed to write data: " + serial->errorString());
        frame.clear();
    } else {

        if (serial->waitForReadyRead(100)) {
                QByteArray data = serial->readAll();
                data = data.mid(1,2);
                ui->data_edt->setText(data.toHex(' ').toUpper());
                frame.clear();
            } else {
                ui->data_edt->setText("Timeout");
            }
        serial->flush();
    }
}




void MainWindow::on_write_clicked()
{
    if (!serial->isOpen()) {
        QMessageBox::warning(this, "Port Closed", "Please connect to a COM port first!");
        return;
    }

    // 3. Convert Hex String to QByteArray
    QByteArray frame;

    // Byte 0: Header
    frame.append(static_cast<char>(m_header));

    // Byte 1 & 2: [R/W bit (1 bit)] + [Address (15 bits)]
    // Pack R/W into the most significant bit of the 16-bit block
    QString addrStr = ui->address_edt->text().trimmed();

        // 2. Convert Hex String to Unsigned Int (Base 16)
        bool ok;
        uint32_t tempAddr = addrStr.toUInt(&ok, 16);

        // 3. Store in the variable if conversion was successful
        if (ok) {
            // Mask to 15 bits as per your frame requirement
            m_address = static_cast<uint16_t>(tempAddr & 0x7FFF);
        } else {
            QMessageBox::warning(this, "Input Error", "Invalid Address! Please enter a Hex value (e.g., 1A2B)");
            return; // Stop the function if address is bad
        }

    QString dataStr = ui->data_edt->text().trimmed();

            // 2. Convert Hex String to Unsigned Int (Base 16)

    m_data = dataStr.toUInt(&ok, 16);

    uint16_t addrBlock = (m_isWrite ? 0x8000 : 0x0000) | m_address;
    frame.append(static_cast<char>((addrBlock >> 8) & 0xFF)); // High Byte
    frame.append(static_cast<char>(addrBlock & 0xFF));        // Low Byte

    // Byte 3 & 4: Data (16 bit)
    frame.append(static_cast<char>((m_data >> 8) & 0xFF));    // MSB
    frame.append(static_cast<char>(m_data & 0xFF));           // LSB

    // Byte 5: XOR Checksum (Calculated over all 5 previous bytes)
    m_checksum = 0;
    for(int i = 0; i < frame.size(); ++i) {
        m_checksum ^= static_cast<uint8_t>(frame.at(i));
    }
    frame.append(static_cast<char>(m_checksum));

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
