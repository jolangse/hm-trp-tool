#include "configurator.h"
#include "ui_creator.h"
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

#include <unistd.h>
#include <termios.h> /* Terminal I/O support */
#include <errno.h>   /* Error number definitions */
#include <fcntl.h>   /* File descriptor manipulation */
#include <stdio.h>
#include <errno.h>

#ifndef CBUFFER_SIZE
#define CBUFFER_SIZE 32
#endif

const int count_devtype = 4;
const char* dev_name[4] = {
    "HM-TRP-433",
    "HM-TRP-470",
    "HM-TRP-868",
    "HM-TRP-915"
};

const uint32_t freq_min[4] = {
    414 * 1e6,
    450 * 1e6,
    849 * 1e6,
    895 * 1e6
};
const uint32_t freq_max[4] = {
    454 * 1e6,
    490 * 1e6,
    889 * 1e6,
    935 * 1e6
};
const uint32_t freq_default[4] = {
    434 * 1e6,
    470 * 1e6,
    869 * 1e6,
    915 * 1e6
};

const int count_recv_bw = 12;
// Values selected somewhat randomly...
const int recv_bw[12] = {
    30,
    50,
    75,
    90,
    105,
    120,
    150,
    175,
    200,
    250,
    300,
    620
};

const int count_freq_dev = 10;
// Values selected somewhat randomly...
const int freq_dev[10] = {  // Values as kHz ..
    10,
    20,
    35,
    50,
    55,
    80,
    110,
    120,
    145,
    160
};

const int count_powerlevel = 8;
const int powerlevel[8] = { 1, 2, 5, 8, 11, 14, 17, 20 };

const int count_rates = 9;
const int port_rate_value[9] = {
    1200,
    1800,
    2400,
    4800,
    9600,
    19200,
    38400,
    57600,
    115200
};

const int port_rates[9] = {
    B1200,
    B1800,
    B2400,
    B4800,
    B9600,
    B19200,
    B38400,
    B57600,
    B115200
};

const char cmd_reset[3]     = { 0xAA, 0xFA, 0xF0 };
const char cmd_config[3]    = { 0xAA, 0xFA, 0xE1 };
const char cmd_frequency[3] = { 0xAA, 0xFA, 0xD2 };
const char cmd_air_rate[3]   = { 0xAA, 0xFA, 0xC3 };
const char cmd_bw[3]        = { 0xAA, 0xFA, 0xB4 };
const char cmd_deviation[3] = { 0xAA, 0xFA, 0xA5 };
const char cmd_power[3]     = { 0xAA, 0xFA, 0x96 };
const char cmd_uart_rate[3] = { 0xAA, 0xFA, 0x1E };
const char cmd_RSSI[3]      = { 0xAA, 0xFA, 0x87 };
const char cmd_SNR[3]       = { 0xAA, 0xFA, 0x78 };

int Configurator::write_uint32_t ( int f, uint32_t v )
{
    int i;
    unsigned char buf[4];
    v = __bswap_32(v);
    for ( i = 0; i < 4; i++ )
        buf[i] = (v>>(8*i) & 0xff );

    return write( f, buf, 4 );
}

int Configurator::write_uint16_t ( int f, uint16_t v )
{
    int i;
    unsigned char buf[2];
    v = __bswap_16(v);
    for ( i = 0; i < 2; i++ )
        buf[i] = (v>>(8*i) & 0xff );

    return write( f, buf, 2 );
}

int Configurator::write_uint8_t ( int f, uint8_t v )
{
    return write( f, &v, 1 );
}

int Configurator::write_cmd ( int f, const char* buf )
{
    return write( f, buf, 3 ); 	// Send the command to the device
}

int Configurator::read_config ( int fd, config_t * config )
{

    unsigned char buf[CBUFFER_SIZE];
    int res;

    res = write_cmd( fd, cmd_config ); 	// Send the command to the device
    if ( res < 0 ) return(-1); 		// and die on failure..

    //int i = 0;
    uint8_t* tmp_config = (uint8_t*)config;
    do {
        bzero(buf, CBUFFER_SIZE);
        res = read( fd, buf, 1 );

        if ( res )
        {
            *tmp_config++ = (uint8_t)buf[0];
            // Make sure wo don't overflow the config struct.
            if ( (uint8_t*)tmp_config > ((uint8_t*)config+sizeof(config_t)) ) return(-2);
        }

    } while ( res > 0 );

    if ( res < 0 )
    {
        return -1;
    }

    if ( config->freq )
    {
        config->freq      = __bswap_32( config->freq );
        config->air_rate  = __bswap_32( config->air_rate );
        config->bw        = __bswap_16( config->bw );
        config->uart_rate = __bswap_32( config->uart_rate );
        return 1;
    }

    return 0; // No errors, but no valid config
}

int Configurator::read_ok ( int fd )
{
    unsigned char ok[5];
    int i = 0;
    unsigned char c;
    bzero(ok, 5);

    while( (read( fd, &c, 1 ) > 0 ) && ( i < 4) ) ok[i++] = c;
    if ( strcmp( (const char*)ok, "OK\r\n" ) == 0 ) return 1;
    return 0;
}


int Configurator::open_port ( const char* device, int rate )
{
    int f;
    struct termios tio;

    f = open ( device, O_RDWR | O_NOCTTY ); // Not using O_NDELAY, that caused ERR11: temp unavail.
    if ( f < 0 )
    {
        return(-1);
    }

    tio.c_cflag = rate;

    // 8 bits, local mode, read enabled
    tio.c_cflag |= CS8 | CLOCAL | CREAD;

    // no flow-control,
    tio.c_cflag &= ~CRTSCTS;

    // no parity
        tio.c_iflag = IGNPAR;

        tio.c_oflag = 0;
        tio.c_lflag = 0; // Non-canonical read, no other options.
    tio.c_cc[VMIN] = 0; // Disable minimum characters for read
        tio.c_cc[VTIME] = 5; // Wait for a max of 0.5 second

        tcsetattr(f,TCSANOW,&tio); // Apply this now.

    tcflush(f, TCIOFLUSH); //  the serial input buffer.

    return f;
}


Configurator::Configurator(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Configurator)
{

    ui->setupUi(this);

    config = (config_t*)malloc(sizeof(config_t));
    bzero(config, sizeof(config_t));
    connect(ui->serialPort, SIGNAL(returnPressed()), this, SLOT(on_readButton_clicked()));

}

Configurator::~Configurator()
{
    delete ui;
}

void Configurator::on_readButton_clicked()
{
    ui->statusBar->showMessage("Looking for device...");

    ui->serialPort->setFocus();

    ui->readButton->setEnabled(false);
    ui->writeButton->setEnabled(false);
    ui->defaultsButton->setEnabled(false);
    update();
    repaint();
    QCoreApplication::processEvents();


    QByteArray byteArray = (ui->serialPort->text()).toLocal8Bit();
    const char* device = byteArray.constData();

    bzero(config, sizeof(config_t));

    for ( int r = 0; r <= count_rates; r++ )
    {
        cur_rate_i = port_rates[r];
        int fd = open_port( device, cur_rate_i );
        if ( fd < 0 )
        {
            ui->statusBar->showMessage( QString("Failed to open port: ") + QString ( strerror( errno )) );
            ui->readButton->setEnabled(true);
            return;
        }

        if ( read_config(fd, config) < 0 )
        {
            ui->statusBar->showMessage( QString("Failed to read config: ") + QString ( strerror( errno )) );
            ui->readButton->setEnabled(true);
            return;
        }
        if ( read_config( fd, config )  == 1 )
        {
            ui->statusBar->showMessage( QString("Found device at ") + QString().setNum( port_rate_value[r] ) );
            printf("Read Verify-rate: %d\n", cur_rate_i);

            if ( (config->air_rate < (uint32_t)port_rates[0])   ||
                    (config->uart_rate < (uint32_t)port_rates[0])  ||
                    (config->bw < (uint16_t)recv_bw[0] )           ||
                    (config->deviation < (uint8_t)freq_dev[0] ) )
            {
                QMessageBox::critical(this, QString("Power cycle device!"),
                                      QString("The data returned by the connected ") + QString( dev_name[dev_model] ) +  QString(" is invalid.\nPlease power-cycle your device by e.g. disconnecting\nand reconnectingbefore proceeding.")
                                      );
            }

            printf("Freq      %d \n", config->freq );
            printf("Air rate  %d \n", config->air_rate );
            printf("Deviation %d \n", config->deviation );
            printf("TX Power  %d \n", config->power );
            printf("BW        %d \n", config->bw );
            printf("UART rate %d \n", config->uart_rate );

            ui->curRate->setText( QString().setNum( port_rate_value[r] ) );

            for ( int i = 0; i < count_devtype; i++ )
            {
                if ( (config->freq >= freq_min[i]) &&
                        (config->freq <= freq_max[i]))
                { dev_model = i; break; }
            }

            ui->devType->setText( QString( dev_name[dev_model] ) );

            ui->airRate->clear();
            for ( int i = 0; i < count_rates; i++ )
                ui->airRate->insertItem(i, QString().setNum( port_rate_value[i] ) );

            ui->uartRate->clear();
            for ( int i = 0; i < count_rates; i++ )
                ui->uartRate->insertItem(i, QString().setNum( port_rate_value[i] ) );

            ui->recvBandwidth->clear();
            for ( int i = 0; i < count_recv_bw; i++ )
                ui->recvBandwidth->insertItem(i, QString().setNum( recv_bw[i] ) );

            ui->freqDeviation->clear();
            for ( int i = 0; i < count_freq_dev; i++ )
                ui->freqDeviation->insertItem(i, QString().setNum( freq_dev[i] ) );

            ui->powerLevel->clear();
            for ( int i = 0; i < count_powerlevel; i++ )
                ui->powerLevel->insertItem(i, QString().setNum( powerlevel[i] ) );

            for ( int i = 0; i < count_rates; i++ )
                if ( config->air_rate == (uint32_t)port_rate_value[i] )
                    ui->airRate->setCurrentIndex(i);

            for ( int i = 0; i < count_rates; i++ )
                if ( config->uart_rate == (uint32_t)port_rate_value[i] )
                    ui->uartRate->setCurrentIndex(i);

            for ( int i = 0; i < count_recv_bw; i++ )
                if ( config->bw == (uint16_t)recv_bw[i] )
                    ui->recvBandwidth->setCurrentIndex(i);

            for ( int i = 0; i < count_freq_dev; i++ )
                if ( config->deviation == (uint8_t)freq_dev[i] )
                    ui->freqDeviation->setCurrentIndex(i);

            for ( int i = 0; i < count_powerlevel; i++ )
                if ( powerlevel[config->power] == (uint8_t)powerlevel[i] )
                    ui->powerLevel->setCurrentIndex(i);

            ui->frequency->setText( QString().setNum( config->freq/1000 ) );

            ui->readButton->setEnabled(true);
            ui->writeButton->setEnabled(true);
            ui->defaultsButton->setEnabled(true);

            ::close(fd);
            return;
        }
        ::close(fd);
    }

    ui->statusBar->showMessage( QString("Unable to locate device!"), 10000);
    QMessageBox::critical(this, QString("Unable to locate device"),
                          QString("Could not find a connected HM-TRP.\nPlease verify that the CONFIG pin is jumpered LOW,\nand power-cycle your device by e.g. disconnecting\nand reconnecting before proceeding.")
                          );

    ui->readButton->setEnabled(true);

}


void Configurator::on_writeButton_clicked()
{

    ui->serialPort->setFocus();
    ui->statusBar->showMessage( QString("Writing settings to device ...") );

    ui->readButton->setEnabled(false);
    ui->writeButton->setEnabled(false);
    ui->defaultsButton->setEnabled(false);
    update();
    repaint();
    QCoreApplication::processEvents();

    QByteArray byteArray = (ui->serialPort->text()).toLocal8Bit();
    const char* device = byteArray.constData();

    int fd = open_port( device, cur_rate_i );
    if ( fd < 0 )
    {
        ui->statusBar->showMessage( QString("Failed to open port: ") + QString ( strerror( errno )) );
        ui->readButton->setEnabled(true);
        return;
    }

    if ( read_config(fd, config) != 1 )
    {
        ui->statusBar->showMessage( QString("Failed to verify device config: ") + QString ( strerror( errno )) );
        ui->readButton->setEnabled(true);
        return;
    }

    uint32_t freq = (ui->frequency->text()).toInt() * 1000;
    if (

            ( freq < (uint32_t)freq_min[dev_model] ) ||
            ( freq > (uint32_t)freq_max[dev_model] ) )
    {
        QMessageBox::critical(this, QString("Invalid frequency"),
                              QString("The given frequency is outside of the permitted range\n") +
                              QString("Device type ") + QString( dev_name[dev_model] ) + QString(" permits frequencies\n") +
                              QString("from ") + QString().setNum( freq_min[dev_model]/1000) + QString("kHz to ") + QString().setNum( freq_max[dev_model]/1000) + QString("kHz.")
                              );
        return;
    }

    config->air_rate = ui->airRate->currentText().toInt();
    config->bw = ui->recvBandwidth->currentText().toInt();
    config->deviation = ui->freqDeviation->currentText().toInt();
    config->freq = freq;
    config->uart_rate = ui->uartRate->currentText().toInt();




    for ( int i = 0; i < count_powerlevel; i++ )
    {

        if ( ui->powerLevel->currentText().toInt() == powerlevel[i] )
        {
            config->power = i;
            break;
        }
    }

    printf("After config changes:");
    printf("Freq      %d \n", config->freq );
    printf("Air rate  %d \n", config->air_rate );
    printf("Deviation %d \n", config->deviation );
    printf("TX Power  %d \n", config->power );
    printf("BW        %d \n", config->bw );
    printf("UART rate %d \n", config->uart_rate );

    write_cmd( fd, cmd_frequency );
    write_uint32_t( fd,  config->freq );
    if ( !read_ok( fd ) )
        return errorStatus("Failed to set frequency");

    write_cmd( fd, cmd_air_rate );
    write_uint32_t( fd,  config->air_rate );
    if ( !read_ok( fd ) )
        return errorStatus("Failed to set air data rate");

    write_cmd( fd, cmd_bw );
    write_uint16_t( fd,  config->bw );
    if ( !read_ok( fd ) )
        return errorStatus("Failed to set recieve bandwidth");

    write_cmd( fd, cmd_deviation );
    write_uint8_t( fd,  config->deviation );
    if ( !read_ok( fd ) )
        return errorStatus("Failed to set frequency deviation");

    write_cmd( fd, cmd_power );
    write_uint8_t( fd,  config->power );
    if ( !read_ok( fd ) )
        return errorStatus("Failed to set transmit power");

    write_cmd( fd, cmd_uart_rate );
    write_uint32_t( fd, config->uart_rate );
    read_ok( fd ); // Clear buffer.

    ::close(fd);

    on_readButton_clicked();

}

void Configurator::on_defaultsButton_clicked()
{

    ui->serialPort->setFocus();

    ui->readButton->setEnabled(false);
    ui->writeButton->setEnabled(false);
    ui->defaultsButton->setEnabled(false);
    update();
    repaint();
    QCoreApplication::processEvents();

    QByteArray byteArray = (ui->serialPort->text()).toLocal8Bit();
    const char* device = byteArray.constData();

    int fd = open_port( device, cur_rate_i );
    if ( fd < 0 )
    {
        ui->statusBar->showMessage( QString("Failed to open port: ") + QString ( strerror( errno )) );
        ui->readButton->setEnabled(true);
        return;
    }

    if ( read_config(fd, config) != 1 )
    {
        ui->statusBar->showMessage( QString("Failed to verify device config: ") + QString ( strerror( errno )) );
        ui->readButton->setEnabled(true);
        return;
    }
    write_cmd( fd, cmd_reset );
    ::close(fd);

    QMessageBox::critical(this, QString("Power cycle device!"),
                          QString("The reset command has been sent to the ") + QString( dev_name[dev_model] ) +  QString(".\nPlease power-cycle your device by e.g. disconnecting\nand reconnectingbefore proceeding.")
                          );

    on_readButton_clicked();

}

void Configurator::errorStatus(QString message)
{
    ui->statusBar->showMessage( message );
    ui->readButton->setEnabled(true);
    ui->writeButton->setEnabled(true);
    ui->defaultsButton->setEnabled(true);
}
