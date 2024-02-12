import { Component } from '@angular/core';
import { TranslateService } from '@ngx-translate/core';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss']
})
export class AppComponent {
  title = 'Winderoo';

  constructor(translate: TranslateService) {
    // Fallback language
    translate.setDefaultLang('en');
    
    // The language to use if the desired one isn't available
    translate.use('en');
  }
}
