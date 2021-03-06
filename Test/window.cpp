#include "pch.h"
#include "window.h"

Window::Window(QWidget *parent) : QWidget(parent)
{
    #ifdef Q_OS_WIN 
    setFixedSize(800, 600);
    #endif
    setupUi(this);
}

Window::~Window()
{
   delete m_shopManager;
}

void Window::showEvent(QShowEvent *ev)
{
   QWidget::showEvent(ev);
   if (m_shopManager == nullptr)
   {
      m_shopManager = new ShopManager(this->windowHandle(), this);

      connect(btnSubscribe, &QPushButton::clicked, this, &Window::purchaseSubscription);
      connect(btnBuyProduct, &QPushButton::clicked, this, &Window::purchaseProduct);
      connect(btnBuyDurable, &QPushButton::clicked, this, &Window::purchaseDurable);
      connect(btnCheckSubscription, &QPushButton::clicked, this, &Window::checkSubscription);
      connect(btnCheckDurable, &QPushButton::clicked, this, &Window::checkDurable);
      connect(&MyLogger::instance(), &MyLogger::writeLog, this, &Window::writeLog);
	  connect(m_shopManager, &ShopManager::isTrial, this, &Window::handleTrial);
	  connect(m_shopManager, &ShopManager::isActive, this, &Window::handleActive);
	  connect(m_shopManager, &ShopManager::isDurableActive, this, &Window::handleDurableActive);
	  connect(m_shopManager, &ShopManager::isSubscriptionActive, this, &Window::handleSubscriptionActive);

	  m_shopManager->initShop();
   }
}

void Window::handleError(const QString& errorMessage)
{
   QMessageBox msgBox;
   msgBox.setText(errorMessage);
   msgBox.exec();
}
void Window::handleTrial(bool isTrial)
{
	if (isTrial)
		markAsProductTrial();
}
void Window::handleActive(bool isActive)
{
	if (isActive)
		markAsProductPurchased();

}
void Window::handleDurableActive(bool isPurchased)
{
	if(isPurchased)
		markAsDurablePurchased();
}
void Window::handleSubscriptionActive(bool isActive)
{
	if (isActive)
		markAsSubscribed();
}
void Window::purchaseDurable()
{
    m_shopManager->doPurchase(ShopManager::banana_once_product);
}

void Window::purchaseSubscription()
{
    m_shopManager->doPurchase(ShopManager::banana_subscription);
}

void Window::purchaseProduct()
{
	m_shopManager->doPurchase(ShopManager::banana_product);
}

void Window::checkSubscription()
{
	m_shopManager->checkSubscription();
}

void Window::checkDurable()
{
	m_shopManager->checkDurable();
}


void Window::markAsSubscribed()
{
    lblSubscription->setText("Subscribed");
	btnSubscribe->setEnabled(false);
    this->hide();
    this->show();
}


void Window::markAsDurablePurchased()
{
    lblDurable->setText("Durable purchased");
	btnBuyDurable->setEnabled(false);
    this->hide();
    this->show();
}

void Window::markAsProductPurchased()
{
	lblTrial->setText("is purchased");
	btnBuyProduct->setEnabled(false);
	this->hide();
	this->show();
	lblIsTrial->setStyleSheet("QLabel { color : green; }");
}

void Window::markAsProductTrial()
{
	lblTrial->setText("is trial");
	btnBuyProduct->setEnabled(true);
	this->hide();
	this->show();
	lblIsTrial->setStyleSheet("QLabel { color : red; }");
}

void Window::writeLog(QString log) {
	txtLog->append(log);
}
