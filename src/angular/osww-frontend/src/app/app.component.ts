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
    translate.setDefaultLang('en-US');

    // Check if we have a prefered language, if not set English
    const selectedLanguage = localStorage.getItem('selectedLanguage');
    translate.use(selectedLanguage ?? 'en-US');
  }
}
