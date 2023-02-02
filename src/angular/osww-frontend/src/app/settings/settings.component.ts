import { Component, OnInit } from '@angular/core';
import { ApiService, Update } from '../api.service';

@Component({
  selector: 'app-settings',
  templateUrl: './settings.component.html',
  styleUrls: ['./settings.component.scss']
})
export class SettingsComponent implements OnInit {

  wifiSignalIcon = ''
  batteryStrength = '';
  
  upload = {
    activityState: '',
    statusMessage: '',
    disabled: false,
    direction: '',
    rpd: 0
  }

  watchWindingParametersURL = 'https://watch-winder.store/watch-winding-table/';

  constructor(private apiService: ApiService) { }

  ngOnInit(): void {
    this.upload.statusMessage = 'Save Settings'

    this.apiService.getStatus().subscribe((data) => {
      this.upload.activityState = data.status;
      this.upload.rpd = data.rotationsPerDay;
      this.upload.direction = data.direction;
      this.wifiSignalIcon = this.getWifiSignalStrengthIcon(data.db *= -1);
      this.batteryStrength = this.getBatteryStrengthIcon(data.batteryLevel);  // @todo - add battery data
    })
  }


  setRotationsPerDay(rpd: any) {
    this.upload.rpd = rpd.value
  }

  getColour(status: string): string {
    switch (status) {
      case 'Winding':
        return 'green';
      case 'Stopped':
        return 'red';
      case 'Paused':
        return 'yellow';
      default:
        return 'no-status';
    }
  }

  getWifiSignalStrengthIcon(db: number) {
    if (db <= 30) {
      return 'wifi';
    }
    if (db >= 31 && db <= 60) {
      return 'wifi_2_bar';
    }
    if(db >= 61 && db <= 90) {
      return 'wifi_2_bar';
    }
    if( db >= 91 && db <= 120) {
      return 'wifi_1_bar';
    }
    return 'wifi_off';
  }

  getBatteryStrengthIcon(db: any) {
    if (db <= 30) {
      return 'battery_4_bar';
    }
    if (db >= 31 && db <= 60) {
      return 'battery_4_bar';
      }
    if(db >= 61 && db <= 90) {
      return 'battery_2_bar';
    }
    if( db >= 91 && db <= 120) {
      return 'battery_0_bar';
    }
    return 'battery_unknown';
  }

  getReadableDirectionOfRotation(direction: string): string {
    switch (direction) {
      case 'CW':
        return 'Clockwise';
      case 'CCW':
        return 'Counter-Clockwise';
      case 'BOTH':
        return 'Both';
      default:
        return 'Clockwise';
    }
  }

  getActivityStateForAPI(activityState: string): string {
    switch (activityState) {
      case 'Winding':
        return 'START';
      case 'Stopped':
        return 'STOP';
      default:
        return 'STOP';
    }
  }

  openSite(URL: string) {
    window.open(URL, '_blank');
  }

  uploadSettings(actionToDo?: string) {
    this.upload.statusMessage = 'Upload in progress';
    this.upload.disabled = true;

    const body: Update = {
      action: actionToDo ? actionToDo : this.getActivityStateForAPI(this.upload.activityState),
      rotationDirection: this.upload.direction,
      tpd: this.upload.rpd,
    }

    this.apiService.updateState(body).subscribe((response) => {
      if (response.status == 204) {
        this.apiService.getStatus().subscribe((data) => {
          this.upload.activityState = data.status;
          this.upload.rpd = data.rotationsPerDay;
          this.upload.direction = data.direction;
        })
      }
      this.upload.disabled = false;
      this.upload.statusMessage = 'Save Settings';
    });
  }

  beginProgramming() {
    this.uploadSettings('START');
  }

  stopProgramming() {
    this.uploadSettings('STOP');
  }

}
