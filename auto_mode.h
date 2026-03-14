#ifndef AUTO_MODE_H
#define AUTO_MODE_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>

namespace Ui {
class auto_mode;
}

class auto_mode : public QMainWindow
{
    Q_OBJECT

public:
    explicit auto_mode(QWidget *parent = nullptr);
    ~auto_mode();

private slots:

    void on_cn_tx_cnfg_clicked();

    void on_lvttl_tx_en_clicked(bool checked);

    void on_rs422_tx_en_clicked(bool checked);

    void on_tx_clicked(bool checked);

    void on_lvds_tx_en_clicked(bool checked);

    void on_rs422_tx_cnfg_clicked();

    void on_lvttl_tx_cnfg_clicked();

    void on_lvds_rx_en_clicked(bool checked);

    void on_rs422_rx_en_clicked(bool checked);

    void on_lvttl_rx_en_clicked(bool checked);

    void on_rx_clicked(bool checked);

    void on_ref_clicked();

    void on_connect_clicked();

    void on_read_clicked();

    void on_write_clicked();

    void uart_write(uint16_t address, uint16_t data, bool isWrite);

    quint16 uart_read(uint16_t address);

    void on_freq_cnfg_clicked();

    void on_tx_enable_clicked();

    void on_lvds_rx_read_clicked();

    void on_tx_config_clicked();

    void on_lvttl_rx_read_clicked();


private:
    Ui::auto_mode *ui;
    QSerialPort *serial;
    std::vector<uint16_t>
    gpi_grp1_pul_cnt_addr_lst,
    gpi_grp2_pul_cnt_addr_lst,
    gpi_grp3_pul_cnt_addr_lst,
    gpi_grp4_pul_cnt_addr_lst,
    gpi_grp5_pul_cnt_addr_lst,

    gpi_grp1_glitch_cnt_addr_lst,
    gpi_grp2_glitch_cnt_addr_lst,
    gpi_grp3_glitch_cnt_addr_lst,
    gpi_grp4_glitch_cnt_addr_lst,
    gpi_grp5_glitch_cnt_addr_lst;
};

#endif // AUTO_MODE_H
