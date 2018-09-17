// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>
#include "ElaUtils.h"
#include "ISubWallet.h"
#include "nlohmann/json.hpp"

using namespace Elastos::ElaWallet;

#define  CLASS_SUBWALLET   "com/elastos/spvcore/ISubWallet"
#define  FIELD_SUBWALLET   "mSubProxy"

static jstring JNICALL nativeGetChainId(JNIEnv *env, jobject clazz, jlong jSubProxy)
{
    ISubWallet* subWallet = (ISubWallet*)jSubProxy;
    std::string result = subWallet->GetChainId();
    return env->NewStringUTF(result.c_str());
}

static jstring JNICALL nativeGetBasicInfo(JNIEnv *env, jobject clazz, jlong jSubProxy)
{
	ISubWallet* subWallet = (ISubWallet*)jSubProxy;
	nlohmann::json result = subWallet->GetBasicInfo();
	return env->NewStringUTF(result.dump().c_str());
}

static jstring JNICALL nativeGetBalanceInfo(JNIEnv *env, jobject clazz, jlong jSubProxy)
{
    ISubWallet* subWallet = (ISubWallet*)jSubProxy;
    nlohmann::json result = subWallet->GetBalanceInfo();
    return env->NewStringUTF(result.dump().c_str());
}

static jlong JNICALL nativeGetBalance(JNIEnv *env, jobject clazz, jlong jSubProxy)
{
    ISubWallet* subWallet = (ISubWallet*)jSubProxy;
    return (jlong)subWallet->GetBalance();
}

static jstring JNICALL nativeCreateAddress(JNIEnv *env, jobject clazz, jlong jSubProxy)
{
    ISubWallet* subWallet = (ISubWallet*)jSubProxy;
    std::string result = subWallet->CreateAddress();
    return env->NewStringUTF(result.c_str());
}

static jstring JNICALL nativeGetAllAddress(JNIEnv *env, jobject clazz, jlong jSubProxy, jint jStart, jint jCount)
{
    ISubWallet* subWallet = (ISubWallet*)jSubProxy;
    nlohmann::json addresses = subWallet->GetAllAddress(jStart, jCount);
    return env->NewStringUTF(addresses.dump().c_str());
}

static jlong JNICALL nativeGetBalanceWithAddress(JNIEnv *env, jobject clazz, jlong jSubProxy, jstring jaddress)
{
    const char* address = env->GetStringUTFChars(jaddress, NULL);

    ISubWallet* subWallet = (ISubWallet*)jSubProxy;
    uint64_t result = subWallet->GetBalanceWithAddress(address);

    env->ReleaseStringUTFChars(jaddress, address);
    return (jlong)result;
}

class ElaSubWalletCallback: public ISubWalletCallback
{
public:
    virtual void OnTransactionStatusChanged(
        const std::string &txid,
        const std::string &status,
        const nlohmann::json &desc,
        uint32_t confirms);

    virtual void OnBlockSyncStarted();

    /**
     * Callback method fired when best block chain height increased. This callback could be used to show progress.
     * @param currentBlockHeight is the of current block when callback fired.
     * @param progress is current progress when block height increased.
     */
    virtual void OnBlockHeightIncreased(uint32_t currentBlockHeight, double progress);

    /**
     * Callback method fired when block end synchronizing with a peer. This callback could be used to show progress.
     */
    virtual void OnBlockSyncStopped();

	/**
	 * Callback method fired when subwallet was destroyed.
	 */
	virtual void OnDestroyWallet();

    ElaSubWalletCallback(
        /* [in] */ JNIEnv* env,
        /* [in] */ jobject jobj);

    ~ElaSubWalletCallback();

private:
    JNIEnv* GetEnv();
    void Detach();

private:
    JavaVM* mVM;
    jobject mObj;
};


static std::map<jobject, ElaSubWalletCallback*> sSubCallbackMap;
static void JNICALL nativeAddCallback(JNIEnv *env, jobject clazz, jlong jSubProxy, jobject jsubCallback)
{
    LOGD("FUNC=[%s]========================LINE=[%d]", __FUNCTION__, __LINE__);
    ElaSubWalletCallback* subCallback = new ElaSubWalletCallback(env, jsubCallback);
    ISubWallet* subWallet = (ISubWallet*)jSubProxy;
    subWallet->AddCallback(subCallback);
    sSubCallbackMap[jsubCallback] = subCallback;
}

static void JNICALL nativeRemoveCallback(JNIEnv *env, jobject clazz, jlong jSubProxy, jobject jsubCallback)
{
    ISubWallet* subWallet = (ISubWallet*)jSubProxy;
    std::map<jobject, ElaSubWalletCallback*>::iterator it;
    for (it = sSubCallbackMap.begin(); it != sSubCallbackMap.end(); it++) {
        if (jsubCallback == it->first) {
            subWallet->RemoveCallback(it->second);
            delete it->second;
            sSubCallbackMap.erase(it);
            break;
        }
    }
}

static jstring JNICALL nativeCreateTransaction(JNIEnv *env, jobject clazz, jlong jSubProxy, jstring jfromAddress,
        jstring jtoAddress, jlong amount, jstring jmemo, jstring jremark)
{
    const char* fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char* toAddress = env->GetStringUTFChars(jtoAddress, NULL);
    const char* memo = env->GetStringUTFChars(jmemo, NULL);
    const char* remark = env->GetStringUTFChars(jremark, NULL);

    ISubWallet* subWallet = (ISubWallet*)jSubProxy;
    LOGD("nativeCreateTransaction == fromAddress=[%s], to=[%s], amount=[%lld], memo=[%s]", fromAddress, toAddress, amount, memo);

    nlohmann::json result;
    try {
        result = subWallet->CreateTransaction(fromAddress, toAddress, amount, memo, remark);
    } catch (std::exception& e) {
        ThrowWalletException(env, e.what());
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jtoAddress, toAddress);
    env->ReleaseStringUTFChars(jmemo, memo);
    env->ReleaseStringUTFChars(jremark, remark);

    return env->NewStringUTF(result.dump().c_str());
}

//"(JLjava/lang/String;Ljava/lang/String;JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;"
static jstring JNICALL nativeCreateMultiSignTransaction(JNIEnv *env, jobject clazz, jlong jSubProxy, jstring jfromAddress,
        jstring jtoAddress, jlong amount, jstring jmemo)
{
    const char* fromAddress = env->GetStringUTFChars(jfromAddress, NULL);
    const char* toAddress = env->GetStringUTFChars(jtoAddress, NULL);
    const char* memo = env->GetStringUTFChars(jmemo, NULL);

    ISubWallet* subWallet = (ISubWallet*)jSubProxy;
    nlohmann::json result;

    try {
        result = subWallet->CreateMultiSignTransaction(fromAddress, toAddress, amount, memo);
    }
    catch (std::invalid_argument& e) {
        ThrowWalletException(env, e.what());
    }
    catch (std::logic_error& e) {
        ThrowWalletException(env, e.what());
    }
    catch (std::runtime_error& e) {
        ThrowWalletException(env, e.what());
    }
    catch (std::exception& e) {
        ThrowWalletException(env, e.what());
    }

    env->ReleaseStringUTFChars(jfromAddress, fromAddress);
    env->ReleaseStringUTFChars(jtoAddress, toAddress);
    env->ReleaseStringUTFChars(jmemo, memo);

    return env->NewStringUTF(result.dump().c_str());
}

static jstring JNICALL nativeAppendSignToTransaction(JNIEnv *env, jobject clazz, jlong jSubProxy,
					jstring jRawTransaction,
					jstring jPayPassword)
{
	const char* rawTransaction = env->GetStringUTFChars(jRawTransaction, NULL);
	const char* payPassword = env->GetStringUTFChars(jPayPassword, NULL);

	ISubWallet* subWallet = (ISubWallet*)jSubProxy;
	nlohmann::json tx = std::string(rawTransaction);
	nlohmann::json result;

	try {
		result = subWallet->AppendSignToMultiSignTransaction(tx, payPassword);
	} catch (std::exception& e) {
		ThrowWalletException(env, e.what());
	}

	env->ReleaseStringUTFChars(jRawTransaction, rawTransaction);
	env->ReleaseStringUTFChars(jPayPassword, payPassword);

	return env->NewStringUTF(result.dump().c_str());
}

static jstring JNICALL nativePublishMultiSignTransaction(JNIEnv *env, jobject clazz, jlong jSubProxy,
					jstring jRawTransaction,
					jlong fee)
{
	const char* rawTransaction = env->GetStringUTFChars(jRawTransaction, NULL);

	ISubWallet* subWallet = (ISubWallet*)jSubProxy;
	nlohmann::json result;

	try {
		result = subWallet->PublishMultiSignTransaction(std::string(rawTransaction), fee);
	} catch (std::exception& e) {
		ThrowWalletException(env, e.what());
	}

	env->ReleaseStringUTFChars(jRawTransaction, rawTransaction);

	return env->NewStringUTF(result.dump().c_str());
}

static jlong JNICALL nativeCalculateTransactionFee(JNIEnv *env, jobject clazz, jlong jSubProxy, jstring jrawTransaction, jlong feePerKb)
{
    const char* rawTransaction = env->GetStringUTFChars(jrawTransaction, NULL);
    ISubWallet* subWallet = (ISubWallet*)jSubProxy;
    long fee = 0;

    try {
        fee = subWallet->CalculateTransactionFee(std::string(rawTransaction), feePerKb);
    } catch (std::exception& e) {
        ThrowWalletException(env, e.what());
    }

    env->ReleaseStringUTFChars(jrawTransaction, rawTransaction);

    return (jlong)fee;
}

static jstring JNICALL nativeSendRawTransaction(JNIEnv *env, jobject clazz, jlong jSubProxy,
		jstring jTransactionJson,
		jlong jFee,
		jstring jPayPassword)
{
    const char* transactionJson = env->GetStringUTFChars(jTransactionJson, NULL);
    const char* payPassword = env->GetStringUTFChars(jPayPassword, NULL);

    ISubWallet* subWallet = (ISubWallet*)jSubProxy;
    nlohmann::json result;

    try {
        result = subWallet->SendRawTransaction(std::string(transactionJson), jFee, payPassword);
        env->ReleaseStringUTFChars(jTransactionJson, transactionJson);
        env->ReleaseStringUTFChars(jPayPassword, payPassword);
        return env->NewStringUTF(result.dump().c_str());
    } catch (std::exception& e) {
        ThrowWalletException(env, e.what());
    }

    env->ReleaseStringUTFChars(jTransactionJson, transactionJson);
    env->ReleaseStringUTFChars(jPayPassword, payPassword);

    return NULL;
}

static jstring JNICALL nativeGetAllTransaction(JNIEnv *env, jobject clazz, jlong jSubProxy, jint start,
        jint count, jstring jaddressOrTxid)
{
    const char* addressOrTxid = env->GetStringUTFChars(jaddressOrTxid, NULL);

    ISubWallet* subWallet = (ISubWallet*)jSubProxy;
    LOGD("Func=[%s]===Line=[%d]", __FUNCTION__, __LINE__);
    nlohmann::json result = subWallet->GetAllTransaction(start, count, addressOrTxid);

    LOGD("Func=[%s]===Line=[%d]", __FUNCTION__, __LINE__);
    env->ReleaseStringUTFChars(jaddressOrTxid, addressOrTxid);
    jstring value = env->NewStringUTF(result.dump().c_str());
    LOGD("Func=[%s]===Line=[%d]", __FUNCTION__, __LINE__);
    return value;
}


static jstring JNICALL nativeSign(JNIEnv *env, jobject clazz, jlong jSubProxy, jstring jmessage, jstring jpayPassword)
{
    const char* message = env->GetStringUTFChars(jmessage, NULL);
    const char* payPassword = env->GetStringUTFChars(jpayPassword, NULL);

    ISubWallet* subWallet = (ISubWallet*)jSubProxy;
    std::string result;

    try {
        result = subWallet->Sign(message, payPassword);
    }
    catch (std::invalid_argument& e) {
        ThrowWalletException(env, e.what());
    }
    catch (std::logic_error& e) {
        ThrowWalletException(env, e.what());
    }
    catch (std::runtime_error& e) {
        ThrowWalletException(env, e.what());
    }
    catch (std::exception& e) {
        ThrowWalletException(env, e.what());
    }

    env->ReleaseStringUTFChars(jmessage, message);
    env->ReleaseStringUTFChars(jpayPassword, payPassword);
    return env->NewStringUTF(result.c_str());
}

static jstring JNICALL nativeCheckSign(JNIEnv *env, jobject clazz, jlong jSubProxy,
		jstring jPublicKey,
		jstring jMessage,
		jstring jSignature)
{
    const char* publicKey = env->GetStringUTFChars(jPublicKey, NULL);
    const char* message = env->GetStringUTFChars(jMessage, NULL);
    const char* signature = env->GetStringUTFChars(jSignature, NULL);

    ISubWallet* subWallet = (ISubWallet*)jSubProxy;
    nlohmann::json result;
    try {
        result = subWallet->CheckSign(publicKey, message, signature);
    } catch (std::exception& e) {
        ThrowWalletException(env, e.what());
    }

    env->ReleaseStringUTFChars(jPublicKey, publicKey);
    env->ReleaseStringUTFChars(jMessage, message);
    env->ReleaseStringUTFChars(jSignature, signature);

    return env->NewStringUTF(result.dump().c_str());
}

static jstring JNICALL nativeGetPublicKey(JNIEnv *env, jobject clazz, jlong jSubProxy)
{
	ISubWallet* subWallet = (ISubWallet*)jSubProxy;
	std::string result = subWallet->GetPublicKey();
	return env->NewStringUTF(result.c_str());
}

static const JNINativeMethod gMethods[] = {
    {"nativeGetChainId", "(J)Ljava/lang/String;", (void*)nativeGetChainId},
    {"nativeGetBasicInfo", "(J)Ljava/lang/String;", (void*)nativeGetBasicInfo},
    {"nativeGetBalanceInfo", "(J)Ljava/lang/String;", (void*)nativeGetBalanceInfo},
    {"nativeGetBalance", "(J)J", (void*)nativeGetBalance},
    {"nativeCreateAddress", "(J)Ljava/lang/String;", (void*)nativeCreateAddress},
    {"nativeGetAllAddress", "(JII)Ljava/lang/String;", (void*)nativeGetAllAddress},
    {"nativeGetBalanceWithAddress", "(JLjava/lang/String;)J", (void*)nativeGetBalanceWithAddress},
    {"nativeAddCallback", "(JLcom/elastos/spvcore/ISubWalletCallback;)V", (void*)nativeAddCallback},
    {"nativeRemoveCallback", "(JLcom/elastos/spvcore/ISubWalletCallback;)V", (void*)nativeRemoveCallback},
    {"nativeCreateTransaction", "(JLjava/lang/String;Ljava/lang/String;JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;", (void*)nativeCreateTransaction},
    {"nativeCreateMultiSignTransaction", "(JLjava/lang/String;Ljava/lang/String;JLjava/lang/String;)Ljava/lang/String;", (void*)nativeCreateMultiSignTransaction},
    {"nativeAppendSignToTransaction", "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;", (void*)nativeAppendSignToTransaction},
    {"nativePublishMultiSignTransaction", "(JLjava/lang/String;J)Ljava/lang/String;", (void*)nativePublishMultiSignTransaction},
    {"nativeCalculateTransactionFee", "(JLjava/lang/String;J)J", (void*)nativeCalculateTransactionFee},
    {"nativeSendRawTransaction", "(JLjava/lang/String;JLjava/lang/String;)Ljava/lang/String;", (void*)nativeSendRawTransaction},
    {"nativeGetAllTransaction", "(JIILjava/lang/String;)Ljava/lang/String;", (void*)nativeGetAllTransaction},
    {"nativeSign", "(JLjava/lang/String;Ljava/lang/String;)Ljava/lang/String;", (void*)nativeSign},
    {"nativeCheckSign", "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z", (void*)nativeCheckSign},
    {"nativeGetPublicKey", "(J)Ljava/lang/String;", (void*)nativeGetPublicKey},
};

jint register_elastos_spv_ISubWallet(JNIEnv *env)
{
    return jniRegisterNativeMethods(env, "com/elastos/spvcore/ISubWallet",
        gMethods, NELEM(gMethods));
}

ElaSubWalletCallback::ElaSubWalletCallback(
    /* [in] */ JNIEnv* env,
    /* [in] */ jobject jobj)
{
    LOGD("FUNC=[%s]========================LINE=[%d]", __FUNCTION__, __LINE__);
    mObj = env->NewGlobalRef(jobj);
    env->GetJavaVM(&mVM);
}

ElaSubWalletCallback::~ElaSubWalletCallback()
{
    LOGD("FUNC=[%s]========================LINE=[%d]", __FUNCTION__, __LINE__);
    if (mObj) {
        GetEnv()->DeleteGlobalRef(mObj);
    }
}

JNIEnv* ElaSubWalletCallback::GetEnv()
{
    JNIEnv* env;
    assert(mVM != NULL);
    mVM->AttachCurrentThread(&env, NULL);
    return env;
}

void ElaSubWalletCallback::Detach()
{
    assert(mVM != NULL);
    mVM->DetachCurrentThread();
}

void ElaSubWalletCallback::OnTransactionStatusChanged(const std::string &txid, const std::string &status,
    const nlohmann::json &desc, uint32_t confirms)
{
    JNIEnv* env = GetEnv();
    LOGD("FUNC=[%s]========================LINE=[%d]", __FUNCTION__, __LINE__);

    jclass clazz = env->GetObjectClass(mObj);
    //"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V"
    jmethodID methodId = env->GetMethodID(clazz, "OnTransactionStatusChanged","(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");
    jstring jtxid = env->NewStringUTF(txid.c_str());
    jstring jstatus = env->NewStringUTF(status.c_str());
    jstring jdesc = env->NewStringUTF(desc.dump().c_str());

    env->CallVoidMethod(mObj, methodId, jtxid, jstatus, jdesc, confirms);

    Detach();
}

void ElaSubWalletCallback::OnBlockSyncStarted()
{
    JNIEnv* env = GetEnv();
    LOGD("FUNC=[%s]========================LINE=[%d]", __FUNCTION__, __LINE__);

    jclass clazz = env->GetObjectClass(mObj);
    jmethodID methodId = env->GetMethodID(clazz, "OnBlockSyncStarted","()V");
    env->CallVoidMethod(mObj, methodId);

    Detach();
}

void ElaSubWalletCallback::OnBlockHeightIncreased(uint32_t currentBlockHeight, double progress)
{
    JNIEnv* env = GetEnv();
    LOGD("FUNC=[%s]========================LINE=[%d]", __FUNCTION__, __LINE__);

    jclass clazz = env->GetObjectClass(mObj);
    jmethodID methodId = env->GetMethodID(clazz, "OnBlockHeightIncreased","(ID)V");
    env->CallVoidMethod(mObj, methodId, currentBlockHeight, progress);

    Detach();
}

void ElaSubWalletCallback::OnBlockSyncStopped()
{
    JNIEnv* env = GetEnv();
    LOGD("FUNC=[%s]========================LINE=[%d]", __FUNCTION__, __LINE__);

    jclass clazz = env->GetObjectClass(mObj);
    jmethodID methodId = env->GetMethodID(clazz, "OnBlockSyncStopped","()V");
    env->CallVoidMethod(mObj, methodId);

    Detach();
}

void ElaSubWalletCallback::OnDestroyWallet()
{
	JNIEnv* env = GetEnv();
	LOGD("FUNC=[%s]========================LINE=[%d]", __FUNCTION__, __LINE__);

	jclass clazz = env->GetObjectClass(mObj);
	jmethodID methodId = env->GetMethodID(clazz, "OnDestroyWallet","()V");
	env->CallVoidMethod(mObj, methodId);

	Detach();
}
