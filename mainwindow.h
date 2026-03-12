#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_connect_clicked();

    void on_ref_clicked();

    void on_read_clicked();

    void on_write_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *serial;
    std::vector<uint16_t>
    gp1_grp1_pul_cnt_addr_lst,
    gp1_grp2_pul_cnt_addr_lst,
    gp1_grp3_pul_cnt_addr_lst,
    gp1_grp4_pul_cnt_addr_lst,
    gp1_grp5_pul_cnt_addr_lst,

    gp1_grp1_glitch_cnt_addr_lst,
    gp1_grp2_glitch_cnt_addr_lst,
    gp1_grp3_glitch_cnt_addr_lst,
    gp1_grp4_glitch_cnt_addr_lst,
    gp1_grp5_glitch_cnt_addr_lst
    ;
};

