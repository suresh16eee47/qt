#ifndef AUTO_MODE_H
#define AUTO_MODE_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QButtonGroup>
#include <algorithm>
#include <vector>
#include <QLCDNumber>
#include <QList>

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

    bool uart_write(uint16_t address, uint16_t data, bool isWrite);

    quint16 uart_read(uint16_t address);

    bool on_freq_cnfg_clicked();

    void on_tx_enable_clicked();

    void on_lvds_rx_read_clicked();

    void on_tx_config_clicked();

    void on_lvttl_rx_read_clicked();

    void on_RS422_rx_read_clicked();

    void on_cn_rx1_clicked(bool checked);

    void on_cn_rx2_clicked(bool checked);


    void on_rs422_rx1_clicked(bool checked);

    void on_rs422_rx2_clicked(bool checked);

    void on_rs422_rx3_clicked(bool checked);

    void on_rs422_rx4_clicked(bool checked);

    void action_log(QString message, Qt::GlobalColor color);

    void ui_enable(bool enable);

    void on_read_all_clicked();

    void on_cyc_start_clicked();

    void on_flash_read_clicked();

    void flash_chip_erase();

    uint8_t flash_single_read(uint32_t address,uint16_t data);

    void on_flash_write_clicked();

    void flash_single_write(uint32_t address,uint16_t data);

    void on_flash_chip_erase_clicked();

    void on_action_log_clr_clicked();

    void flash_full_memory_write();

    void on_flash_full_mem_write_clicked();

    QByteArray flash_bulk_read(uint32_t address,uint16_t count);

    QByteArray uart_bulk_read(uint16_t address,uint16_t count);

    void on_uart_bulk_read_clicked();

    void uart_bulk_write(uint16_t address, uint16_t count, QByteArray data);

    void on_uart_bulk_write_clicked();

    void flash_bulk_write(uint32_t address,QByteArray data);

    void on_flash_bulk_write_clicked();

    void flash_full_mem_write();

    QByteArray on_flash_bulk_read_clicked();

    void on_DEBUG_clicked();


private:
    Ui::auto_mode *ui;
    QSerialPort *serial;
    QButtonGroup *lvds_rx_group;
    QButtonGroup *rs422_rx_group;
    QButtonGroup *lvttl_rx_group;
    QButtonGroup *lvds_cn_rx1_group;
    QButtonGroup *lvds_cn_rx2_group;
    QButtonGroup *lvds_cn_rx2_tx_group;
    QButtonGroup *lvds_cn_rx2_tx_dis_group;
    QList<QButtonGroup*> rx_group;
    QList<QButtonGroup*> rx_lvds_group;


    QList<QLCDNumber*> lvds_freq_group;
    QList<QLCDNumber*> lvds_glitch_group;
    QList<QLCDNumber*> rs422_freq_group;
    QList<QLCDNumber*> rs422_glitch_group;
    QList<QLCDNumber*> lvttl_freq_group;
    QList<QLCDNumber*> lvttl_glitch_group;

    QList<QPalette*> lvds_cn_rx1_group_p;
    QList<QPalette*> lvds_cn_rx2_group_p;
    QList<QPalette*> rs422_p;
    QList<QPalette*> lvttl_p;

    std::vector<uint16_t>
    lvds_grp_pul_cnt_addr_lst,
    rs422_grp_pul_cnt_addr_lst,
    lvttl_grp_pul_cnt_addr_lst,
    gpi_grp1_pul_cnt_addr_lst,
    gpi_grp2_pul_cnt_addr_lst,
    gpi_grp3_pul_cnt_addr_lst,
    gpi_grp4_pul_cnt_addr_lst,
    gpi_grp5_pul_cnt_addr_lst,

    gpi_lvds_pul_sts_cnt_addr_lst,
    gpi_rs422_pul_sts_cnt_addr_lst,
    gpi_lvttl_pul_sts_cnt_addr_lst,

    lvds_rx_index,

    lvds_grp_glitch_cnt_addr_lst,
    rs422_grp_glitch_cnt_addr_lst,
    lvttl_grp_glitch_cnt_addr_lst,
    gpi_grp1_glitch_cnt_addr_lst,
    gpi_grp2_glitch_cnt_addr_lst,
    gpi_grp3_glitch_cnt_addr_lst,
    gpi_grp4_glitch_cnt_addr_lst,
    gpi_grp5_glitch_cnt_addr_lst;
};

#endif // AUTO_MODE_H
