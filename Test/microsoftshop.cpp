#include "pch.h"
#include "microsoftshop.h"

void writeLog(QString log) {
	qDebug() << log;
	MyLogger::instance().writeLog(log);
}
winrt::Windows::Services::Store::StoreContext context = nullptr;
void setContext() {
	auto factory = winrt::get_activation_factory<winrt::Windows::Services::Store::StoreContext, winrt::Windows::Services::Store::IStoreContextStatics>();
	context = factory.GetDefault();
}

/* QInAppProduct */
class QInAppProductPrivate
{
public:
	QInAppProductPrivate(const QString &price, const QString &title, const QString &description, QInAppProduct::ProductType type, const QString &id)
		: localPrice(price)
		, localTitle(title)
		, localDescription(description)
		, productType(type)
		, identifier(id)
	{
	}

	QString localPrice;
	QString localTitle;
	QString localDescription;
	QInAppProduct::ProductType productType;
	QString identifier;
};

QInAppProduct::QInAppProduct(const QString &price, const QString &title, const QString &description, ProductType productType, const QString &identifier, QObject *parent)
	: QObject(parent)
{
	d = QSharedPointer<QInAppProductPrivate>(new QInAppProductPrivate(price, title, description, productType, identifier));
}

QString QInAppProduct::identifier() const
{
	return d->identifier;
}

QString QInAppProduct::description() const
{
	return d->localDescription;
}

QString QInAppProduct::title() const
{
	return d->localTitle;
}

QString QInAppProduct::price() const
{
	return d->localPrice;
}

QInAppProduct::ProductType QInAppProduct::productType() const
{
	return d->productType;
}

void QInAppProduct::purchase()
{
	AsyncStore* asyncStore = new AsyncStore();
	asyncStore->setAutoDelete(true);
	asyncStore->setContext(context);
	if(title() == "banana_subscription")
		asyncStore->setOperation(AsyncStore::mType::buySubscription);
	else 
		asyncStore->setOperation(AsyncStore::mType::buyDurable);
	connect(asyncStore, &AsyncStore::appInfo, this, &QInAppProduct::handleStringResponse);
	connect(asyncStore, &AsyncStore::productBought, this, &QInAppProduct::isProductBought);
	QThreadPool::globalInstance()->tryStart(asyncStore);
}


/* QInAppTransaction */
class QInAppTransactionPrivate
{
public:
	QInAppTransactionPrivate(QInAppTransaction::TransactionStatus s,
		QInAppProduct *p)
		: status(s)
		, product(p)
	{
	}

	QInAppTransaction::TransactionStatus status;
	QInAppProduct *product;
};

QInAppTransaction::QInAppTransaction(TransactionStatus status, QInAppProduct *product, QObject *parent) : QObject(parent)
{
	d = QSharedPointer<QInAppTransactionPrivate>(new QInAppTransactionPrivate(status, product));
}

QInAppTransaction::~QInAppTransaction()
{
}

QInAppProduct *QInAppTransaction::product() const
{
	return d->product;
}

QInAppTransaction::TransactionStatus QInAppTransaction::status() const
{
	return d->status;
}

QInAppTransaction::FailureReason QInAppTransaction::failureReason() const
{
	return NoFailure;
}

QString QInAppTransaction::errorString() const
{
	return QString();
}

QDateTime QInAppTransaction::timestamp() const
{
	return QDateTime();
}

QString QInAppTransaction::orderId() const
{
	return QString();
}

QString QInAppTransaction::platformProperty(const QString &propertyName) const
{
	Q_UNUSED(propertyName);
	return QString();
}

/* QInAppStore */
QInAppProduct * QInAppStore::registeredProduct(const QString & id)
{
	QInAppProduct* product;

	if (id == "banana_product")
		product = new QInAppProduct("0", id, id, QInAppProduct::ProductType::Unlockable, "9NDW9G6P5G6X");
	else if (id == "banana_subscription")
		product = new QInAppProduct("0", id, id, QInAppProduct::ProductType::Subscription, "9P2QZFC6NH0M");
	else
		product = new QInAppProduct("0", id, id, QInAppProduct::ProductType::Unlockable, "9P27DZCTDFDR");

	connect(product, &QInAppProduct::handleStringResponse, this, &QInAppStore::handleStringResponse);
	return product;
}


QInAppStore::QInAppStore(QWindow* mainWindow, QObject *parent)
	: QObject(parent)
{
	setContext();
	
	/*StoreContext context = StoreContext.GetDefault();
	IInitializeWithWindow initWindow = (IInitializeWithWindow)(object)context;
	initWindow.Initialize(System.Diagnostics.Process.GetCurrentProcess().MainWindowHandle);	

	ComPtr<IInitializeWithWindow> initWindow;
	hr = storeContext->QueryInterface(IID_PPV_ARGS(&initWindow));
	hr = initWindow->Initialize((HWND)(void*)mainWindow);

	IInitializeWithWindow* initWindow = (IInitializeWithWindow*)(IUnknown*) &context;
	initWindow->Initialize(mainWindow);
	
	IInitializeWithWindow* pIInitWithWindow;
	IInspectable* iInspect = reinterpret_cast<IInspectable*>(&context);
	iInspect->QueryInterface(__uiidd(&pIInitWithWindow), &pIInitWithWindow);
	pIInitWithWindow->Initialize((HWND)(void *)mainWindow);
	pIInitWithWindow->Release();

	winrt::com_ptr<IInitializeWithWindow>* initWindow;
	initWindow = (winrt::com_ptr<IInitializeWithWindow>*)(winrt::com_ptr<IUnknown>*)(&context);
	initWindow->get()->QueryInterface(__uuidof(initWindow), initWindow->put_void());
	initWindow->get()->Initialize((HWND)(void*)mainWindow);
	*/


	winrt::com_ptr<IInitializeWithWindow> initWindow;
	winrt::Windows::Foundation::IUnknown * unknown = reinterpret_cast<winrt::Windows::Foundation::IUnknown*>(&context);
	unknown->as(initWindow);
	initWindow->Initialize((HWND)mainWindow);
	

}

void QInAppStore::checkIsTrial()
{
	AsyncStore* asyncStore = new AsyncStore();
	asyncStore->setAutoDelete(true);
	asyncStore->setContext(context);
	asyncStore->setOperation(AsyncStore::mType::checkIsTrial);
	connect(asyncStore, &AsyncStore::isTrial, this, &QInAppStore::isTrial);
	connect(asyncStore, &AsyncStore::isActive, this, &QInAppStore::isActive);
	connect(asyncStore, &AsyncStore::appInfo, this, &QInAppStore::handleStringResponse);
	QThreadPool::globalInstance()->tryStart(asyncStore);
}


void QInAppStore::getAppInfo()
{
	AsyncStore* asyncStore = new AsyncStore();
	asyncStore->setAutoDelete(true);
	asyncStore->setContext(context);
	asyncStore->setOperation(AsyncStore::mType::getAppInfo);
	connect(asyncStore, &AsyncStore::appInfo, this, &QInAppStore::handleStringResponse);
	QThreadPool::globalInstance()->tryStart(asyncStore);
}

void QInAppStore::getAddonsInfo()
{
	AsyncStore* asyncStore = new AsyncStore();
	asyncStore->setAutoDelete(true);
	asyncStore->setContext(context);
	asyncStore->setOperation(AsyncStore::mType::getAddons);
	connect(asyncStore, &AsyncStore::appInfo, this, &QInAppStore::handleStringResponse);
	QThreadPool::globalInstance()->tryStart(asyncStore);
}

void QInAppStore::getCollectionInfo()
{
	if (context == nullptr) {
		setContext();
	}
	AsyncStore* asyncStore = new AsyncStore();
	asyncStore->setAutoDelete(true);
	asyncStore->setContext(context);
	asyncStore->setOperation(AsyncStore::mType::getUserCollection);
	connect(asyncStore, &AsyncStore::appInfo, this, &QInAppStore::handleStringResponse);
	connect(asyncStore, &AsyncStore::isDurableActive, this, &QInAppStore::isDurableActive);
	connect(asyncStore, &AsyncStore::isSubscriptionActive, this, &QInAppStore::isSubscriptionActive);
	QThreadPool::globalInstance()->tryStart(asyncStore);
}

void QInAppStore::getSubscriptionInfo()
{
	if (context == nullptr) {
		setContext();
	}
	AsyncStore* asyncStore = new AsyncStore();
	asyncStore->setAutoDelete(true);
	asyncStore->setContext(context);
	asyncStore->setOperation(AsyncStore::mType::checkSubscription);
	connect(asyncStore, &AsyncStore::appInfo, this, &QInAppStore::handleStringResponse);
	connect(asyncStore, &AsyncStore::isSubscriptionActive, this, &QInAppStore::isSubscriptionActive);
	QThreadPool::globalInstance()->tryStart(asyncStore);
}

void QInAppStore::getDurableInfo()
{
	if (context == nullptr) {
		setContext();
	}
	AsyncStore* asyncStore = new AsyncStore();
	asyncStore->setAutoDelete(true);
	asyncStore->setContext(context);
	asyncStore->setOperation(AsyncStore::mType::checkDurable);
	connect(asyncStore, &AsyncStore::appInfo, this, &QInAppStore::handleStringResponse);
	connect(asyncStore, &AsyncStore::isDurableActive, this, &QInAppStore::isDurableActive);
	QThreadPool::globalInstance()->tryStart(asyncStore);
}
