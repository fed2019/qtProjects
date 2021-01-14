#ifndef SUBWINSETTING_H
#define SUBWINSETTING_H

#include <QWidget>

namespace Ui {
class SubWinSetting;
}

class SubWinSetting : public QWidget
{
    Q_OBJECT

public:
    explicit SubWinSetting(QWidget *parent = nullptr);
    ~SubWinSetting();
    QStringList getNessaryInfo();
    QStringList getUnNessaryInfo();
    void onBtnConfirmClicked();
    void onBtnCancelClicked();
    void closeEvent(QCloseEvent *) override;

signals:
    void confirmClicked();


private:
    void initView();
    Ui::SubWinSetting *ui;
};

#endif // SUBWINSETTING_H
