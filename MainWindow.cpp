#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "load_metrics.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initial population of the metrics view on startup
    updateMetricsDisplay(currentLoadBalancer);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_refreshButton_clicked()
{
    // Triggered when the user clicks the "Refresh" button in the GUI
    updateMetricsDisplay(currentLoadBalancer);
}

void MainWindow::updateMetricsDisplay(const LoadBalancer& lb)
{
    // 1. Instantiate your metrics class
    metrics systemMetrics;

    // 2. Compute metrics using your exact backend function
    systemMetrics.calculateMetrics(lb);

    // 3. Extract core statistics using your exact getters
    int accepted = systemMetrics.getAcceptedRequests();
    int rejected = systemMetrics.getRejectedRequests();
    double utilization = systemMetrics.getOverallUtilization();

    // 4. Find most and least loaded servers using your class methods
    EdgeServer* mostLoaded = systemMetrics.findMostLoadedServer(lb);
    EdgeServer* leastLoaded = systemMetrics.findLeastLoadedServer(lb);

    // 5. Push data to GUI widgets
    ui->labelAcceptedCount->setText(QString::number(accepted));
    ui->labelRejectedCount->setText(QString::number(rejected));
    
    // Assuming you have a progress bar for system utilization (0-100%)
    ui->progressBarUtilization->setValue(static_cast<int>(utilization));
    ui->labelUtilizationText->setText(QString::number(utilization, 'f', 2) + "%");

    // Display Most Loaded Server details safely
    if (mostLoaded != nullptr) {
        QString serverInfo = QString("Load: %1 / %2")
            .arg(mostLoaded->getcurr_load())
            .arg(mostLoaded->getcapacity());
        ui->labelMostLoadedServer->setText(serverInfo);
    } else {
        ui->labelMostLoadedServer->setText("No servers active");
    }

    // Display Least Loaded Server details safely
    if (leastLoaded != nullptr) {
        QString serverInfo = QString("Load: %1 / %2")
            .arg(leastLoaded->getcurr_load())
            .arg(leastLoaded->getcapacity());
        ui->labelLeastLoadedServer->setText(serverInfo);
    } else {
        ui->labelLeastLoadedServer->setText("No servers active");
    }
}