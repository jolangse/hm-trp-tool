#ifndef CREATOR_H
#define CREATOR_H

#include <QMainWindow>
#include <stdint.h>

typedef struct {
    uint32_t freq;
    uint32_t air_rate;
    uint16_t bw;
    uint8_t  deviation;
    uint8_t  power;
    uint32_t uart_rate;
} config_t;

namespace Ui {
class Configurator;
}

class Configurator : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Configurator(QWidget *parent = 0);
    ~Configurator();
    
private slots:
    void on_readButton_clicked();

    void on_writeButton_clicked();

    void on_defaultsButton_clicked();

private:
    config_t* config;
    int dev_model;
    int cur_rate_i;
    int write_uint32_t ( int f, uint32_t v );
    int write_uint16_t ( int f, uint16_t v );
    int write_uint8_t ( int f, uint8_t v );
    int write_cmd ( int f, const char* buf );
    int read_config ( int fd, config_t * config );
    int read_ok( int fd );
    int open_port ( const char* device, int rate );
    void errorStatus(QString message);

    Ui::Configurator *ui;
};

#endif // CREATOR_H
