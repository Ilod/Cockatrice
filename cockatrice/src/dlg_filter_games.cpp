#include "dlg_filter_games.h"
#include <QDebug>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QSettings>
#include <QCryptographicHash>

DlgFilterGames::DlgFilterGames(const QMap<int, QString> &_allGameTypes, GamesProxyModel *_gamesProxyModel, QWidget *parent)
    : QDialog(parent),
      allGameTypes(_allGameTypes),
      gamesProxyModel(_gamesProxyModel)
{
    QSettings settings;
    settings.beginGroup("filter_games");

    unavailableGamesVisibleCheckBox = new QCheckBox(tr("Show &unavailable games"));
    qDebug() << "getUnavailableGamesVisible() == " << gamesProxyModel->getUnavailableGamesVisible();
    unavailableGamesVisibleCheckBox->setChecked(gamesProxyModel->getUnavailableGamesVisible());

    passwordProtectedGamesVisibleCheckBox = new QCheckBox(tr("Show &password protected games"));
    passwordProtectedGamesVisibleCheckBox->setChecked(gamesProxyModel->getPasswordProtectedGamesVisible());

    gameNameFilterEdit = new QLineEdit;
    gameNameFilterEdit->setText(gamesProxyModel->getGameNameFilter());
    QLabel *gameNameFilterLabel = new QLabel(tr("Game &description:"));
    gameNameFilterLabel->setBuddy(gameNameFilterEdit);

    creatorNameFilterEdit = new QLineEdit;
    creatorNameFilterEdit->setText(gamesProxyModel->getCreatorNameFilter());
    QLabel *creatorNameFilterLabel = new QLabel(tr("&Creator name:"));
    creatorNameFilterLabel->setBuddy(creatorNameFilterEdit);

    QVBoxLayout *gameTypeFilterLayout = new QVBoxLayout;
    QMapIterator<int, QString> gameTypesIterator(allGameTypes);
    while (gameTypesIterator.hasNext()) {
        gameTypesIterator.next();

        QCheckBox *temp = new QCheckBox(gameTypesIterator.value());
        temp->setChecked(gamesProxyModel->getGameTypeFilter().contains(gameTypesIterator.key()));

        gameTypeFilterCheckBoxes.insert(gameTypesIterator.key(), temp);
        gameTypeFilterLayout->addWidget(temp);
    }
    QGroupBox *gameTypeFilterGroupBox;
    if (!allGameTypes.isEmpty()) {
        gameTypeFilterGroupBox = new QGroupBox(tr("&Game types"));
        gameTypeFilterGroupBox->setLayout(gameTypeFilterLayout);
    } else
        gameTypeFilterGroupBox = 0;

    QLabel *maxPlayersFilterMinLabel = new QLabel(tr("at &least:"));
    maxPlayersFilterMinSpinBox = new QSpinBox;
    maxPlayersFilterMinSpinBox->setMinimum(1);
    maxPlayersFilterMinSpinBox->setMaximum(99);
    maxPlayersFilterMinSpinBox->setValue(gamesProxyModel->getMaxPlayersFilterMin());
    maxPlayersFilterMinLabel->setBuddy(maxPlayersFilterMinSpinBox);

    QLabel *maxPlayersFilterMaxLabel = new QLabel(tr("at &most:"));
    maxPlayersFilterMaxSpinBox = new QSpinBox;
    maxPlayersFilterMaxSpinBox->setMinimum(1);
    maxPlayersFilterMaxSpinBox->setMaximum(99);
    maxPlayersFilterMaxSpinBox->setValue(gamesProxyModel->getMaxPlayersFilterMax());
    maxPlayersFilterMaxLabel->setBuddy(maxPlayersFilterMaxSpinBox);

    QGridLayout *maxPlayersFilterLayout = new QGridLayout;
    maxPlayersFilterLayout->addWidget(maxPlayersFilterMinLabel, 0, 0);
    maxPlayersFilterLayout->addWidget(maxPlayersFilterMinSpinBox, 0, 1);
    maxPlayersFilterLayout->addWidget(maxPlayersFilterMaxLabel, 1, 0);
    maxPlayersFilterLayout->addWidget(maxPlayersFilterMaxSpinBox, 1, 1);

    QGroupBox *maxPlayersGroupBox = new QGroupBox(tr("Maximum player count"));
    maxPlayersGroupBox->setLayout(maxPlayersFilterLayout);

    QGridLayout *leftGrid = new QGridLayout;
    leftGrid->addWidget(gameNameFilterLabel, 0, 0);
    leftGrid->addWidget(gameNameFilterEdit, 0, 1);
    leftGrid->addWidget(creatorNameFilterLabel, 1, 0);
    leftGrid->addWidget(creatorNameFilterEdit, 1, 1);
    leftGrid->addWidget(maxPlayersGroupBox, 2, 0, 1, 2);
    leftGrid->addWidget(unavailableGamesVisibleCheckBox, 3, 0, 1, 2);
    leftGrid->addWidget(passwordProtectedGamesVisibleCheckBox, 4, 0, 1, 2);

    QVBoxLayout *leftColumn = new QVBoxLayout;
    leftColumn->addLayout(leftGrid);
    leftColumn->addStretch();

    QVBoxLayout *rightColumn = new QVBoxLayout;
    rightColumn->addWidget(gameTypeFilterGroupBox);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addLayout(leftColumn);
    hbox->addLayout(rightColumn);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hbox);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Filter games"));
}

void DlgFilterGames::actOk() {
    QSettings settings;
    settings.beginGroup("filter_games");
    settings.setValue(
        "unavailable_games_visible",
	unavailableGamesVisibleCheckBox->isChecked()
    );
    settings.setValue(
        "password_protected_games_visible",
        passwordProtectedGamesVisibleCheckBox->isChecked()
    );
    settings.setValue("game_name_filter", gameNameFilterEdit->text());
    settings.setValue("creator_name_filter", creatorNameFilterEdit->text());

    QMapIterator<int, QString> gameTypeIterator(allGameTypes);
    QMapIterator<int, QCheckBox *> checkboxIterator(gameTypeFilterCheckBoxes);
    while (gameTypeIterator.hasNext()) {
        gameTypeIterator.next();
        checkboxIterator.next();

        settings.setValue(
            "game_type/" + hashGameType(gameTypeIterator.value()),
            checkboxIterator.value()->isChecked()
        );
    }

    settings.setValue("min_players", maxPlayersFilterMinSpinBox->value());
    settings.setValue("max_players", maxPlayersFilterMaxSpinBox->value());

    accept();
}

QString DlgFilterGames::hashGameType(const QString &gameType) const {
    return QCryptographicHash::hash(gameType.toUtf8(), QCryptographicHash::Md5).toHex();
}

bool DlgFilterGames::getUnavailableGamesVisible() const
{
    return unavailableGamesVisibleCheckBox->isChecked();
}

void DlgFilterGames::setUnavailableGamesVisible(bool _unavailableGamesVisible)
{
    unavailableGamesVisibleCheckBox->setChecked(_unavailableGamesVisible);
}

bool DlgFilterGames::getPasswordProtectedGamesVisible() const
{
    return passwordProtectedGamesVisibleCheckBox->isChecked();
}

void DlgFilterGames::setPasswordProtectedGamesVisible(bool _passwordProtectedGamesVisible)
{
    passwordProtectedGamesVisibleCheckBox->setChecked(_passwordProtectedGamesVisible);
}

QString DlgFilterGames::getGameNameFilter() const
{
    return gameNameFilterEdit->text();
}

void DlgFilterGames::setGameNameFilter(const QString &_gameNameFilter)
{
    gameNameFilterEdit->setText(_gameNameFilter);
}

QString DlgFilterGames::getCreatorNameFilter() const
{
    return creatorNameFilterEdit->text();
}

void DlgFilterGames::setCreatorNameFilter(const QString &_creatorNameFilter)
{
    creatorNameFilterEdit->setText(_creatorNameFilter);
}

QSet<int> DlgFilterGames::getGameTypeFilter() const
{
    QSet<int> result;
    QMapIterator<int, QCheckBox *> i(gameTypeFilterCheckBoxes);
    while (i.hasNext()) {
        i.next();
        if (i.value()->isChecked())
            result.insert(i.key());
    }
    return result;
}

void DlgFilterGames::setGameTypeFilter(const QSet<int> &_gameTypeFilter)
{
    QMapIterator<int, QCheckBox *> i(gameTypeFilterCheckBoxes);
    while (i.hasNext()) {
        i.next();
        i.value()->setChecked(_gameTypeFilter.contains(i.key()));
    }
}

int DlgFilterGames::getMaxPlayersFilterMin() const
{
    return maxPlayersFilterMinSpinBox->value();
}

int DlgFilterGames::getMaxPlayersFilterMax() const
{
    return maxPlayersFilterMaxSpinBox->value();
}

void DlgFilterGames::setMaxPlayersFilter(int _maxPlayersFilterMin, int _maxPlayersFilterMax)
{
    maxPlayersFilterMinSpinBox->setValue(_maxPlayersFilterMin);
    maxPlayersFilterMaxSpinBox->setValue(_maxPlayersFilterMax == -1 ? maxPlayersFilterMaxSpinBox->maximum() : _maxPlayersFilterMax);
}
