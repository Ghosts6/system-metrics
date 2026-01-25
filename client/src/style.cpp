#include "style.h"

QString Style::getDarkGreenStyleSheet()
{
    return R"(
        QMainWindow {
            background-color: #1a1a1a;
            color: #e0e0e0;
        }
        
        QWidget {
            background-color: #1a1a1a;
            color: #e0e0e0;
        }
        
        QTabWidget::pane {
            border: 1px solid #2d2d2d;
            background-color: #1a1a1a;
            border-radius: 4px;
        }
        
        QTabBar::tab {
            background-color: #2d2d2d;
            color: #b0b0b0;
            padding: 10px 20px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        
        QTabBar::tab:selected {
            background-color: #0d7377;
            color: #ffffff;
            font-weight: bold;
        }
        
        QTabBar::tab:hover {
            background-color: #14a085;
        }
        
        QLabel {
            color: #e0e0e0;
            background-color: transparent;
        }
        
        QLineEdit {
            background-color: #2d2d2d;
            border: 2px solid #3d3d3d;
            border-radius: 4px;
            padding: 6px;
            color: #e0e0e0;
            selection-background-color: #0d7377;
        }
        
        QLineEdit:focus {
            border: 2px solid #0d7377;
        }
        
        QTableWidget {
            background-color: #1a1a1a;
            alternate-background-color: #252525;
            color: #e0e0e0;
            gridline-color: #3d3d3d;
            border: 1px solid #2d2d2d;
            border-radius: 4px;
        }
        
        QTableWidget::item {
            padding: 4px;
        }
        
        QTableWidget::item:selected {
            background-color: #0d7377;
            color: #ffffff;
        }
        
        QHeaderView::section {
            background-color: #2d2d2d;
            color: #e0e0e0;
            padding: 8px;
            border: none;
            font-weight: bold;
        }
        
        QComboBox {
            background-color: #2d2d2d;
            border: 2px solid #3d3d3d;
            border-radius: 4px;
            padding: 6px 8px;
            padding-right: 30px;
            color: #e0e0e0;
            min-width: 120px;
            font-size: 12px;
            selection-background-color: #0d7377;
            selection-color: #ffffff;
        }
        
        QComboBox:hover {
            border: 2px solid #0d7377;
            background-color: #333333;
        }
        
        QComboBox:focus {
            border: 2px solid #14a085;
            background-color: #2d2d2d;
        }
        
        QComboBox:disabled {
            background-color: #1a1a1a;
            color: #808080;
            border: 2px solid #2d2d2d;
        }
        
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 25px;
            border-left: 1px solid #3d3d3d;
            border-top-right-radius: 4px;
            border-bottom-right-radius: 4px;
            background-color: #2d2d2d;
        }
        
        QComboBox::drop-down:hover {
            background-color: #333333;
        }
        
        QComboBox::down-arrow {
            width: 12px;
            height: 12px;
            image: none;
            border-left: 3px solid transparent;
            border-right: 3px solid transparent;
            border-top: 5px solid #e0e0e0;
        }
        
        QComboBox::down-arrow:hover {
            border-top-color: #14a085;
        }
        
        QComboBox QAbstractItemView {
            background-color: #2d2d2d;
            color: #e0e0e0;
            selection-background-color: #0d7377;
            selection-color: #ffffff;
            border: 1px solid #3d3d3d;
            border-radius: 4px;
            padding: 2px;
            outline: none;
        }
        
        QComboBox QAbstractItemView::item {
            padding: 6px 8px;
            border-radius: 2px;
            min-height: 20px;
        }
        
        QComboBox QAbstractItemView::item:hover {
            background-color: #14a085;
            color: #ffffff;
        }
        
        QComboBox QAbstractItemView::item:selected {
            background-color: #0d7377;
            color: #ffffff;
        }
        
        QStatusBar {
            background-color: #1a1a1a;
            color: #0d7377;
            border-top: 1px solid #2d2d2d;
        }
    )";
}

QString Style::getButtonStyle()
{
    return R"(
        QPushButton {
            background-color: #0d7377;
            color: #ffffff;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-weight: bold;
            font-size: 12px;
            min-width: 120px;
        }
        
        QPushButton:hover {
            background-color: #14a085;
        }
        
        QPushButton:pressed {
            background-color: #0a5d61;
            padding: 11px 19px;
        }
        
        QPushButton:disabled {
            background-color: #3d3d3d;
            color: #808080;
        }
    )";
}

QString Style::getProgressBarStyle()
{
    return R"(
        QProgressBar {
            border: 2px solid #2d2d2d;
            border-radius: 8px;
            text-align: center;
            background-color: #2d2d2d;
            color: #e0e0e0;
            font-weight: bold;
            height: 25px;
        }
        
        QProgressBar::chunk {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #0d7377, stop:0.5 #14a085, stop:1 #0d7377);
            border-radius: 6px;
        }
    )";
}

QString Style::getTableStyle()
{
    return R"(
        QTableWidget {
            background-color: #1a1a1a;
            alternate-background-color: #252525;
            color: #e0e0e0;
            gridline-color: #3d3d3d;
            border: 1px solid #2d2d2d;
            border-radius: 4px;
        }
    )";
}

QString Style::getLabelStyle()
{
    return R"(
        QLabel {
            color: #e0e0e0;
            background-color: transparent;
        }
    )";
}
