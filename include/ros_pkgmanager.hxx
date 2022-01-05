/*! **************************************************************************
 * @file ros_packager.hxx
 * @author K. Zarebski
 * @date 2022-01-05
 * @brief Main application definition file
 *
 * This file contains code for the construction of the main GUI application
 * which forms the central interface for the install/uninstall and creation
 * of padckages for Railway Operation Simulator
 * 
 ****************************************************************************/
#ifndef __ROSPKGMANAGER_HXX__
#define __ROSPKGMANAGER_HXX__

#include <QWidget>
#include <QFileDialog>
#include <QTableWidget>
#include <QtMath>
#include <QDebug>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QtCore>
#include <QLabel>
#include <QComboBox>
#include <QMap>
#include <QLabel>
#include <QPushButton>

#include "ros_system.hxx"
#include "rostools/country_codes.hxx"

/*! **********************************************************************
 * @namespace ROSPkg
 * @brief Contains all methods and classes relating to the ROSPkgManager
 ************************************************************************/
namespace ROSPkg {
/*! **********************************************************************
 * @class Manager
 * @brief Constructs the main GUI interface for the package manager
 * 
 * Creates the windows and widgets with which the user interacts acting
 * as the central interface for the install/uninstall and creation of
 * packages. GUI is built using Qt widgets consisting of Buttons,
 * Text Entry objects and windows
 * **********************************************************************/
class Manager : public QWidget
{
    Q_OBJECT

private:
    const int WINDOW_WIDTH = 600;
    const int WINDOW_HEIGHT = 480;
    const int TABLE_WIDTH = 510;
    const int TABLE_HEIGHT = 330;
    const int TABLE_NCOLS = 4;
    const int BUTTON_WIDTH = 150;
    const int BUTTON_HEIGHT = 30;
    QString cache_str_ = "";
    System* system_ = new System(this);
    QWidget* subwindow_ = new QWidget(this, Qt::Tool);
    QSpinBox* year_box_;
    QCheckBox* factual_box_;
    QLabel* factual_box_label_;
    QComboBox* country_code_;
    QLabel* year_box_label_;
    QLabel* country_code_label_;
    QMap<QString, QLineEdit*> package_form_entry_;
    QMap<QString, QLabel*> package_form_labels_;
    QMap<QString, QPushButton*> buttons_ = {
        {"install", new QPushButton(QPushButton::tr("Install Package"), this)},
        {"create", new QPushButton(QPushButton::tr("Create Package"), this)},
        {"uninstall", new QPushButton(QPushButton::tr("Uninstall Package"), this)},
    };
    QLabel* info_str_ = new QLabel(this);
    QTableWidget* installed_ = new QTableWidget(this);

/*! **************************************************************************
 * @brief Populate the table display with all installed packages
 * **************************************************************************/
    void populateTable_();

/*! **************************************************************************
 * @brief Construct the package creation form for metadata entry
 * **************************************************************************/
    void buildPackageForm_();

/*! **************************************************************************
 * @brief Wipe all inputs from the package creation form
 * **************************************************************************/
    void clearPackageForm_();

/*! **************************************************************************
 * @brief Validate information provided in the package creation form
 *
 * @return A mapping of the recorded form information
 * **************************************************************************/
    QMap<QString,QString> checkPackageForm_();
private slots:
/*! **************************************************************************
 * @brief Action associated with the "Install Package" button
 * **************************************************************************/
    void on_InstallButtonClicked();

/*! **************************************************************************
 * @brief Action associated with the "Uninstall Package" button
 * **************************************************************************/
    void on_UninstallButtonClicked();

/*! **************************************************************************
 * @brief Action associated with the "Create Package" button
 * **************************************************************************/
    void on_CreateButtonClicked();

/*! **************************************************************************
 * @brief Action associated with the "Create Package/Confirm" button
 * **************************************************************************/
    void on_CreateConfirmClicked();

/*! **************************************************************************
 * @brief Action associated with the "Create Package/Cancel" button
 * **************************************************************************/
    void on_CreateCancelClicked();

/*! **************************************************************************
 * @brief Action associated with the "Create Package/RLY Browse" button
 * **************************************************************************/
    void on_BrowseRlyFilesClicked();

/*! **************************************************************************
 * @brief Action associated with the "Create Package/TTB Browse" button
 * **************************************************************************/
    void on_BrowseTTBFilesClicked();

/*! **************************************************************************
 * @brief Action associated with the "Create Package/SSN Browse" button
 * **************************************************************************/
    void on_BrowseSSNFilesClicked();

/*! **************************************************************************
 * @brief Action associated with the "Create Package/Docs Browse" button
 * **************************************************************************/
    void on_BrowseDocFilesClicked();
public:
/*! **************************************************************************
 * @brief Initialise an instance of the application GUI definition
 * **************************************************************************/
    Manager();

/*! **************************************************************************
 * @brief Destructor for application GUI definition
 * **************************************************************************/
    ~Manager();
};
};
#endif // ROSPKGMANAGER_HXX
