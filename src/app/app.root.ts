import { Component } from '@angular/core';
import { Platform } from 'ionic-angular';
import { StatusBar } from '@ionic-native/status-bar';
import { SplashScreen } from '@ionic-native/splash-screen';
import {BaseComponent} from './BaseComponent';
import {HeaderComponent, Header} from './header/app.header';
import {TabsComponent} from "../pages/tabs/tabs.component";

@Component({
  selector: 'root',
  templateUrl: './app.root.html'
})
export class RootComponent{
  rootPage:any = TabsComponent;


}
