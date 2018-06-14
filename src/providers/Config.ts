import {Injectable} from '@angular/core';





/***
 * 封装配置信息
 */
@Injectable()
export class Config {

  public static COIN_LIST = {"ELA": {id: 0, name: 'ELA', balance: 0}};

  public static MANAGER_LIST = [{name: '备份钱包'}, {name: '导入钱包'}, {name: '钱包详情'}, {name: '退出钱包'}]

  public static WalletKey = "wallet";

  public static LIST_COUNT = 20;

  //短信接口签名私钥
  private static smsSecretKey = "fe8a5dfa85a0eb2ae7b29c9e4e5d0774c68302971ca2d96bb9d6dd1375abf41d";

  public  static test: any = '';

  private static kycObj:any={};

  public static getKycObj(){
       return this.kycObj;
  }

  public static setKycObj(obj){
        this.kycObj = obj;
  }

  public static getPersonObj(){
       return {
       'id':"",
       'createType':1,
       'status':'0',
       'mnemonic':"zzzzzzz",//助记词
       'priKey':"",
       'backupPassword':"",
       'payPassword':"",
       'maxNumber':3,
       'pubKey':'sdfghhjjkkj',
       'updateTiem':'2018-05-20',
       'finshNumber':0,
       'validateType':[]
      };
  }


  public static getCompanyObj(){
    return {
    'id':"",
    'createType':2,
    'mnemonic':"zzzzzzz",//助记词
    'status':'0',
    'priKey':"",
    'backupPassword':"",
    'payPassword':"",
    'maxNumber':1,
    'pubKey':'sdfghhjjkkj',
    'updateTiem':'2018-05-20',
    'finshNumber':0,
    'validateType':[]
   };
  }

  public static getSmsSecretKey(){
      return this.smsSecretKey;
  }

}


