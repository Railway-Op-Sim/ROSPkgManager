#include "ros_system.hxx"

void ROSPkg::System::createCache_() {
    const QString cache_dir_ = QFileInfo(cache_file_).absolutePath();
    if(!QDir(cache_dir_).exists()) {
        QDir().mkdir(cache_dir_);
    }
    ros_loc_ = QFileDialog::getOpenFileName(
        parent_,
        QFileDialog::tr("Locate") + QString(" Railway Operation Simulator"),
        QString(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)), QFileDialog::tr("ROS Exe (railway.exe)")
    );
    if(ros_loc_.isEmpty() || ros_loc_.isNull()) return;
    QFile file_(cache_file_);
    if(file_.open(QIODevice::WriteOnly)) {
        QTextStream stream_(&file_);
        stream_ << QFileInfo(ros_loc_).absolutePath();
    }
}

ROSPkg::System::System(QWidget* parent) {
    parent_ = parent;

    if(ros_loc_.isEmpty() && QFile::exists(cache_file_)) {
        QFile file_(cache_file_);
        if(file_.open(QIODevice::ReadOnly)) {
            ros_loc_ = file_.readLine();
        }
    }

    while(!QFile::exists(ros_loc_)) {
        createCache_();
        QFile file_(cache_file_);
        if(file_.open(QIODevice::ReadOnly)) {
            ros_loc_ = file_.readLine();
        }
    }

    populateInstalled();
}

void ROSPkg::System::parseMetaFile_(const QString& file_name) {
    //TODO: Need a better way of handling missing informartion

    if(!QFile::exists(file_name)) {
        return;
    }

    const ROSTools::Metadata meta_data_(std::filesystem::path(file_name.toStdString()));

    QFile file_(file_name);
    QString hash_ = "";
    if(file_.open(QIODevice::ReadOnly)) {
        const QString toml_dat_ = file_.readLine();
        hash_ = QString::fromStdString(digestpp::sha256().absorb(toml_dat_.toStdString()).hexdigest());
    }

    qDebug() << "Registering add-on: " << hash_;

    installed_.insert(hash_, meta_data_);

}

void ROSPkg::System::populateInstalled() {
    QDir meta_dir_(ros_loc_ + QDir::separator() + "Metadata");
    QList<QString> filters_ = {"*.toml"};
    QList<QString> toml_files_;
    QDirIterator iter_toml_(meta_dir_.path(), filters_, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    while(iter_toml_.hasNext()) {
        toml_files_ << iter_toml_.next();
    }

    for(const QString& file_ : toml_files_) {
        parseMetaFile_(file_);
    }
}

QList<QList<QTableWidgetItem*>> ROSPkg::System::getTableInfo() const {
    QList<QList<QTableWidgetItem*>> info_;

    for(auto& table : installed_.toStdMap()) {
        std::stringstream ss;
        ss << table.second.version();
        QList<QTableWidgetItem*> row_;
        row_.push_back(new QTableWidgetItem(QString::fromStdString(table.second.name())));
        row_[row_.size()-1]->setFlags(row_[row_.size()-1]->flags() & ~Qt::ItemIsEditable);
        row_.push_back(new QTableWidgetItem(QString::fromStdString(ss.str())));
        row_[row_.size()-1]->setFlags(row_[row_.size()-1]->flags() & ~Qt::ItemIsEditable);
        row_.push_back(new QTableWidgetItem(QString::fromStdString(table.second.author())));
        row_[row_.size()-1]->setFlags(row_[row_.size()-1]->flags() & ~Qt::ItemIsEditable);
        row_.push_back(new QTableWidgetItem(table.first));
        row_[row_.size()-1]->setFlags(row_[row_.size()-1]->flags() & ~Qt::ItemIsEditable);
        info_.push_back(row_);
    }
    return info_;
}

QMap<QString, QList<QString>> ROSPkg::System::getZipFileListing_(const QString& unzip_directory) const {
    // File path filters
    QList<QString> filter_ssn_{"*.ssn", "*.SSN"};
    QList<QString> filter_rly_{"*.rly", "*.RLY"};
    QList<QString> filter_ttb_{"*.ttb", "*.TTB"};
    QList<QString> filter_toml_{"*.toml"};
    QList<QString> filter_docs_{"*.md", "*.pdf"};

    // File result containers
    QMap<QString, QList<QString>> files_ ={
        {"ssn", {}},
        {"toml", {}},
        {"docs", {}},
        {"ttb", {}},
        {"rly", {}}
    };

    // Directory iterators for each file type
    QDirIterator it_ssn(unzip_directory, filter_ssn_, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QDirIterator it_rly(unzip_directory, filter_rly_, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QDirIterator it_ttb(unzip_directory, filter_ttb_, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QDirIterator it_toml(unzip_directory, filter_toml_, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QDirIterator it_docs(unzip_directory, filter_docs_, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    // For each file type search recursively for the needed file types
    while(it_toml.hasNext()) {
        const QString toml_file_{it_toml.next()};
        files_["toml"] << toml_file_;
    }

    while(it_ssn.hasNext()) {
        const QString ssn_file_{it_ssn.next()};
        files_["ssn"] << ssn_file_;
    }

    while(it_rly.hasNext()) {
        const QString rly_file_{it_rly.next()};
        files_["rly"] << rly_file_;
    }

    while(it_ttb.hasNext()) {
        const QString ttb_file_{it_ttb.next()};
        files_["ttb"] << ttb_file_;
    }

    while(it_docs.hasNext()) {
        const QString docs_file_{it_docs.next()};
        files_["docs"] << docs_file_;
    }

    return files_;
}

void ROSPkg::System::unpackZip_(const QMap<QString, QList<QString>>& files_list) const {
    QString info_text_ = "";
    for(const QString& rly_file : files_list["rly"] ) {
        QString base_name_ = QFileInfo(rly_file).fileName();
        QString new_path_ = ros_loc_ + QDir::separator() + "Railways" + QDir::separator() + base_name_;
        info_text_ += "Added " + new_path_;
        qDebug() << "Copying: " << rly_file << " to " << new_path_;
        QFile(rly_file).copy(new_path_);
    }

    for(const QString& ttb_file : files_list["ttb"] ) {
        QString base_name_ = QFileInfo(ttb_file).fileName();
        QString new_path_ = ros_loc_ + QDir::separator() + "Program timetables" + QDir::separator() + base_name_;
        info_text_ += "\n\nAdded " + new_path_;
        qDebug() << "Copying: " << ttb_file << " to " << new_path_;
        QFile(ttb_file).copy(new_path_);
    }

    for(const QString& ssn_file : files_list["ssn"] ) {
        QString base_name_ = QFileInfo(ssn_file).fileName();
        QString new_path_ = ros_loc_ + QDir::separator() + "Sessions" + QDir::separator() + base_name_;
        info_text_ += "\n\nAdded " + new_path_;
        qDebug() << "Copying: " << ssn_file << " to " << new_path_;
        QFile(ssn_file).copy(new_path_);
    }

    // If the Documentation directory does not yet exist create it
    if(!QDir(ros_loc_ + QDir::separator() + "Documentation").exists()) {
        QDir().mkdir(ros_loc_ + QDir::separator() + "Documentation");
    }

    QString doc_dir_ = ros_loc_ + QDir::separator() + "Documentation";

    ROSTools::Metadata package_data_;

    try {
        package_data_ = ROSTools::Metadata(std::filesystem::path(files_list["toml"][0].toStdString()));
    }
    catch(const std::invalid_argument&) {
        QMessageBox::critical(
            parent_,
            QMessageBox::tr("Missing Package Metadata"),
            QMessageBox::tr("Cannot install selected package, missing package metadata.")
        );
        
        return;
    }
    if(!files_list["docs"].empty()) {
        // Create directory for add-on docs
        doc_dir_  += QDir::separator() + QString::fromStdString(package_data_.display_name()).replace(" ", "_");
        QDir().mkdir(doc_dir_);
    }

    for(const QString& doc_file : files_list["docs"] ) {
        QString base_name_;
        QFileInfo(doc_file).fileName();
        QString new_path_ = doc_dir_ + QDir::separator() + base_name_;
        info_text_ += "\n\nAdded " + new_path_;
        QFile(doc_file).copy(new_path_);
    }

    QMessageBox::information(parent_, QMessageBox::tr("Add-on installed successfully"), QMessageBox::tr(info_text_.toStdString().c_str()));
}

void ROSPkg::System::unzipFile(const QString& file_name) const {
    QTemporaryDir temp_dir_;
    QDir().mkdir(temp_dir_.path());
    elz::extractZip(file_name.toStdString(), temp_dir_.path().toStdString());
    QMap<QString, QList<QString>> files_list_ = getZipFileListing_(temp_dir_.path());

    // If no TOML file is found then archive is not an ROS package yet so one needs to be created
    if(files_list_["toml"].empty()) {

        const QString package_name_ = QString(QFileInfo(file_name).baseName()).replace("_", " ");

        ROSPkg::Packager packager_{parent_, ros_loc_, package_name_};

        if(files_list_["rly"].size() != 1) {
            QMessageBox::critical(
                parent_,
                QMessageBox::tr("Package definition ambiguous"),
                QMessageBox::tr("Expected single RLY file from archive.")
            );
            
            return;
        }

        if(files_list_["ttb"].empty()) {
            QMessageBox::critical(
                parent_,
                QMessageBox::tr("Missing timetables"),
                QMessageBox::tr("Expected one or more TTB files within archive.")
            );
            
            return;
        }

        for(const QString& ttb_file : files_list_["ttb"]) packager_.addTTBFile(ttb_file);

        if(!files_list_["ssn"].empty()) {
            for(const QString& ssn_file : files_list_["ssn"]) packager_.addSSNFile(ssn_file);
        }

        if(!files_list_["docs"].empty()) {
            for(const QString& doc_file : files_list_["docs"]) packager_.addDocFile(doc_file);
        }

        packager_.setRLYFile(QFileInfo(files_list_["rly"][0]).fileName());

        const QString new_toml_ = packager_.buildTOML();

        if(new_toml_.isEmpty()) {
            QMessageBox::critical(
                parent_,
                QMessageBox::tr("TOML creation failure"),
                QMessageBox::tr("TOML creation for non-project archive failed.")
            );
            
            return;
        }

        files_list_["toml"].push_back(new_toml_);

    }
    else if(files_list_["toml"].size() > 1) {
        QMessageBox::critical(
            parent_,
            QMessageBox::tr("Package Definition Ambiguous"),
            QMessageBox::tr("Expected single metadata (TOML) file in package, but multiple candidates found.")
        );
        
        return;
    }

    unpackZip_(files_list_);
}

void ROSPkg::System::uninstall(const QString& sha) {
    QString info_text_ = "";
    const ROSTools::Metadata meta_data_ = installed_[sha];
    const QString data_dir_ = ros_loc_ + QDir::separator();
    const QString toml_file_ = QFileInfo(QString::fromStdString(meta_data_.toml_file().string())).fileName();

    info_text_ += "Removed '" + data_dir_ + "Railways" + QDir::separator() + QString::fromStdString(meta_data_.rly_file().string()) + "'";
    QFile(data_dir_ + "Railways" + QDir::separator() + QString::fromStdString(meta_data_.rly_file().string())).remove();

    info_text_ += "\n\nRemoved '" + data_dir_ + "Metadata" + QDir::separator() + toml_file_ + "'";
    QFile(data_dir_ + "Metadata" + QDir::separator() + toml_file_).remove();

    const QList<QString> ttb_files_;

    for(const std::filesystem::path& ttb_file: meta_data_.ttb_files()){
        info_text_ += "\n\nRemoved '" + data_dir_ + QDir::separator() + "Program timetables" + QDir::separator() + QString::fromStdString(ttb_file.string()) + "'";

        QFile(data_dir_ + QDir::separator() + "Program timetables" + QDir::separator() + QString::fromStdString(ttb_file.string())).remove();
    }

    for(const std::filesystem::path& doc_file: meta_data_.doc_files()){
        info_text_ += "\n\nRemoved '" + data_dir_ + QDir::separator() + "Documentation" + QDir::separator() + QString::fromStdString(doc_file.string()) + "'";

        QFile(data_dir_ + QDir::separator() + "Documentation" + QDir::separator() + QString::fromStdString(doc_file.string())).remove();
    }

    for(const std::filesystem::path& ssn_file: meta_data_.ssn_files()){
        info_text_ += "\n\nRemoved '" + data_dir_ + QDir::separator() + "Sessions" + QDir::separator() + QString::fromStdString(ssn_file.string()) + "'";

        QFile(data_dir_ + QDir::separator() + "Sessions" + QDir::separator() + QString::fromStdString(ssn_file.string())).remove();
    }
    installed_.remove(sha);
    QMessageBox::information(parent_, QMessageBox::tr("Add-on uninstalled successfully"), QMessageBox::tr(info_text_.toStdString().c_str()));
}
