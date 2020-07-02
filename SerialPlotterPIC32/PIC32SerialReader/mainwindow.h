#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QTime>
#include <QElapsedTimer>
#include <QVector>
#include <QVector2D>

#define BUFF_LEN 200
#define PLOT_COUNT 2// Number of seperate data items that are to be plotted

#define UNO_VENDOR_ID 9025
#define UNO_PRODUCT_ID 67
#define PIC32_VENDOR_ID 1027
#define PIC32_PRODUCT_ID 24577

QT_BEGIN_NAMESPACE
namespace Ui
{
      class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


    QByteArray serialData;
    QString serialBuffer;
    QString plottingData;

    QByteArray begin_temp="";
    QByteArray end_temp="";


    QVector<double> data_point1, data_point2, data_index;
    uint64_t counter=0;

    QVector<QVector<double>> data_array;



private slots:

    void on_btn_open_port_clicked();

    void write_serial(QString);//Function to write to serial port

    //int first_index_of(char *,int,char);//Function to return the first index of a char in a string
    //int last_index_of(char *,int, char);//Function to return the last index of a char in a string
    void store_data();//store available data in to the buffer

    //std::string read_data(char *);///reads the stored data from buffer
    void segregate_data(std::string );//plot data after reading from buffer
    //bool buffer_full();

    void plot_data(QVector <double>, QVector<double>, uint8_t);


private:
    Ui::MainWindow *ui;
    QSerialPort *device;
    static const quint16 device_vendor_id=PIC32_VENDOR_ID;
    static const quint16 device_product_id=PIC32_PRODUCT_ID;
    QString device_port_name;//string to store port name
    bool device_is_available;


    };
#endif // MAINWINDOW_H
