#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "loadbalancer.h"
#include "load_metrics.h"

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
    void on_refreshButton_clicked();

private:
    Ui::MainWindow *ui;
    LoadBalancer currentLoadBalancer; // Manages your server instances

    void updateMetricsDisplay(const LoadBalancer& lb);
};

#endif // MAINWINDOW_H