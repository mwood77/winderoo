import { Component, OnInit, ViewContainerRef } from '@angular/core';
import { MatDialog, MatDialogRef } from '@angular/material/dialog';
import { ApiService } from '../api.service';      
import { TranslateService, TranslateModule } from '@ngx-translate/core';
import packageJson from '../../../package.json';

@Component({
  selector: 'app-header',
  templateUrl: './header.component.html',
  styleUrls: ['./header.component.scss']
})
export class HeaderComponent implements OnInit {
  
  isWinderEnabled: boolean;
  isWinderEnabledNum: number;
    
  constructor(private dialog: MatDialog, private apiService: ApiService, public translateService: TranslateService) { 
    this.isWinderEnabledNum = this.apiService.isWinderEnabled$.getValue();
    this.isWinderEnabled = false;
  }

  public changeLanguage(language: string): void {
    this.translateService.use(language);
    localStorage.setItem('selectedLanguage', language);
  }

  private mapEnabledState($event: number): void {
    if ($event == 1) {
      this.isWinderEnabled = true;
    } else {
      this.isWinderEnabled = false;
    };
  }

  ngOnInit(): void {  
    this.apiService.isWinderEnabled$.subscribe((e) => {
      this.isWinderEnabledNum = e;
      this.mapEnabledState(e);
    })
  }

  openDialog(): void {
    this.dialog.open(ResetDialog, {
      width: '80%',
    });
  }

  reset(): void {
    this.openDialog();
  }

  updateWinderEnabledState($state: any) {
    this.apiService.updatePowerState(ApiService.getWindowHref(window), $state).subscribe(
      (data) => {
        this.apiService.isWinderEnabled$.next($state);

        this.mapEnabledState($state)
        // signal settings component to refresh
        this.apiService.shouldRefresh$.next(true);
      });
  };
  
}

@Component({
  selector: 'app-header-dialog',
  templateUrl: './header-dialog.component.html',
  styleUrls: ['./header-dialog.component.scss'],
  standalone: true,
  imports: [TranslateModule],
})
export class ResetDialog {

  version = packageJson.version;

  constructor(public dialogRef: MatDialogRef<ResetDialog>, 
    private apiService: ApiService,
    public translateService: TranslateService) {}
  
  confirmReset(): void {
    this.apiService.resetDevice(ApiService.getWindowHref(window)).subscribe();
  }

  closeDialog(): void {
    this.dialogRef.close()
  }

}