#include "dmpriscontrol.h"
#include "private/dmpriscontrol_p.h"
#include "private/mpris/dmprismonitor.h"

#include <QPainter>
#include <QDebug>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

DMPRISControl::DMPRISControl(QWidget *parent)
    : QFrame(parent),
      DObject(*new DMPRISControlPrivate(this))
{
    D_D(DMPRISControl);

    d->init();
}

bool DMPRISControl::isWorking() const
{
    D_DC(DMPRISControl);

    return d->m_mprisInter;
}

DMPRISControlPrivate::DMPRISControlPrivate(DMPRISControl *q)
    : DObjectPrivate(q),

      m_mprisInter(nullptr)
{
}

void DMPRISControlPrivate::init()
{
    D_Q(DMPRISControl);

    m_mprisMonitor = new DMPRISMonitor(q);

    m_title = new QLabel;
    m_title->setAlignment(Qt::AlignCenter);
    m_prevBtn = new DImageButton;
    m_prevBtn->setObjectName("PrevBtn");
    m_pauseBtn = new DImageButton;
    m_pauseBtn->setObjectName("PauseBtn");
    m_playBtn = new DImageButton;
    m_playBtn->setObjectName("PlayBtn");
    m_nextBtn = new DImageButton;
    m_nextBtn->setObjectName("NextBtn");

#ifdef QT_DEBUG
    m_title->setText("MPRIS Title");
    m_nextBtn->setNormalPic("://images/arrow_right_normal.png");
    m_pauseBtn->setNormalPic("://images/arrow_left_white.png");
    m_playBtn->setNormalPic("://images/arrow_right_white.png");
    m_prevBtn->setNormalPic("://images/arrow_left_normal.png");
#endif

    QHBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->addWidget(m_prevBtn);
    controlLayout->addStretch();
    controlLayout->addWidget(m_pauseBtn);
    controlLayout->addWidget(m_playBtn);
    controlLayout->addStretch();
    controlLayout->addWidget(m_nextBtn);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_title);
    centralLayout->addLayout(controlLayout);

    q->setLayout(centralLayout);

    q->connect(m_mprisMonitor, SIGNAL(mprisAcquired(const QString &)), q, SLOT(_q_loadMPRISPath(const QString &)));
    q->connect(m_mprisMonitor, SIGNAL(mprisLost(const QString &)), q, SLOT(_q_removeMPRISPath(const QString &)));
    q->connect(m_prevBtn, SIGNAL(clicked()), q, SLOT(_q_onPrevClicked()));
    q->connect(m_pauseBtn, SIGNAL(clicked()), q, SLOT(_q_onPauseClicked()));
    q->connect(m_playBtn, SIGNAL(clicked()), q, SLOT(_q_onPlayClicked()));
    q->connect(m_nextBtn, SIGNAL(clicked()), q, SLOT(_q_onNextClicked()));

    m_mprisMonitor->init();
}

void DMPRISControlPrivate::_q_onPrevClicked()
{
    if (!m_mprisInter)
        return;

    m_mprisInter->Previous();
}

void DMPRISControlPrivate::_q_onPlayClicked()
{
    if (!m_mprisInter)
        return;

    m_mprisInter->Play();
}

void DMPRISControlPrivate::_q_onPauseClicked()
{
    if (!m_mprisInter)
        return;

    m_mprisInter->Pause();
}

void DMPRISControlPrivate::_q_onNextClicked()
{
    if (!m_mprisInter)
        return;

    m_mprisInter->Next();
}

void DMPRISControlPrivate::_q_onMetaDataChanged()
{
    if (!m_mprisInter)
        return;

    const auto meta = m_mprisInter->metadata();
    const QString title = meta.value("xesam:title").toString();
    const QString artist = meta.value("xesam:artist").toString();

    if (title.isEmpty())
        m_title->clear();
    else
    {
        if (artist.isEmpty())
            m_title->setText(title);
        else
            m_title->setText(QString("%1 - %2").arg(title).arg(artist));
    }
}

void DMPRISControlPrivate::_q_onPlaybackStatusChanged()
{
    const QString stat = m_mprisInter->playbackStatus();

    if (stat == "Playing")
    {
        m_pauseBtn->setVisible(true);
        m_playBtn->setVisible(false);
    } else {
        m_pauseBtn->setVisible(false);
        m_playBtn->setVisible(true);
    }
}

void DMPRISControlPrivate::_q_loadMPRISPath(const QString &path)
{
    D_Q(DMPRISControl);

    const bool hasOld = m_mprisInter;
    m_lastPath = path;

    if (m_mprisInter)
        m_mprisInter->deleteLater();

    m_mprisInter = new DBusMPRIS(path, "/org/mpris/MediaPlayer2", QDBusConnection::sessionBus(), q);

    q->connect(m_mprisInter, SIGNAL(MetadataChanged(QVariantMap)), q, SLOT(_q_onMetaDataChanged()));
    q->connect(m_mprisInter, SIGNAL(PlaybackStatusChanged(QString)), q, SLOT(_q_onPlaybackStatusChanged()));

    _q_onMetaDataChanged();
    _q_onPlaybackStatusChanged();

    if (hasOld)
        emit q->mprisChanged();
    else
        emit q->mprisAcquired();
}

void DMPRISControlPrivate::_q_removeMPRISPath(const QString &path)
{
    D_QC(DMPRISControl);

    if (m_lastPath != path)
        return;

    if (!m_mprisInter)
        return;

    m_mprisInter->deleteLater();
    m_mprisInter = nullptr;

    emit q->mprisLosted();
}

#include "moc_dmpriscontrol.cpp"
