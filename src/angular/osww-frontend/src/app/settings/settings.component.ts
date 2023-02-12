import { Component, OnInit } from '@angular/core';
import { DateAdapter } from '@angular/material/core';
import { ApiService, Update } from '../api.service';

interface SelectInterface {
  value: string;
  viewValue: string;
}

@Component({
  selector: 'app-settings',
  templateUrl: './settings.component.html',
  styleUrls: ['./settings.component.scss']
})
export class SettingsComponent implements OnInit {

  minutes: SelectInterface[] = [
      { value: '00', viewValue: '00' },
      { value: '10', viewValue: '10' },
      { value: '20', viewValue: '20' },
      { value: '30', viewValue: '30' },
      { value: '40', viewValue: '40' },
      { value: '50', viewValue: '50' },
  ]

  hours: SelectInterface[] = [
      { value: '00', viewValue: '00' },
      { value: '01', viewValue: '01' },
      { value: '02', viewValue: '02' },
      { value: '03', viewValue: '03' },
      { value: '04', viewValue: '04' },
      { value: '05', viewValue: '05' },
      { value: '06', viewValue: '06' },
      { value: '07', viewValue: '07' },
      { value: '08', viewValue: '08' },
      { value: '09', viewValue: '09' },
      { value: '10', viewValue: '10' },
      { value: '11', viewValue: '11' },
      { value: '12', viewValue: '12' },
      { value: '13', viewValue: '13' },
      { value: '14', viewValue: '14' },
      { value: '15', viewValue: '15' },
      { value: '16', viewValue: '16' },
      { value: '17', viewValue: '17' },
      { value: '18', viewValue: '18' },
      { value: '19', viewValue: '19' },
      { value: '20', viewValue: '20' },
      { value: '21', viewValue: '21' },
      { value: '22', viewValue: '22' },
      { value: '23', viewValue: '03' },
  ];

  wifiSignalIcon = ''
  
  upload = {
    activityState: '',
    statusMessage: '',
    disabled: false,
    direction: '',
    rpd: 0,
    hour: '00',
    minutes: '00',
    durationInSecondsToCompleteOneRevolution: 0
  }

  selectedHour: any;
  selectedMinutes: any;
  estDuration: any;

  watchWindingParametersURL = 'https://watch-winder.store/watch-winding-table/';

  constructor(private apiService: ApiService) { }

  ngOnInit(): void {
    this.upload.statusMessage = 'Save Settings'

    this.apiService.getStatus(ApiService.getWindowHref(window)).subscribe((data) => {
      this.upload.activityState = data.status;
      this.upload.rpd = data.rotationsPerDay;
      this.upload.direction = data.direction;
      this.upload.hour = data.hour;
      this.upload.minutes = data.minutes;
      this.wifiSignalIcon = this.getWifiSignalStrengthIcon(data.db *= -1);
      this.upload.durationInSecondsToCompleteOneRevolution = data.durationInSecondsToCompleteOneRevolution;
      
      this.estimateDuration(this.upload.rpd);
    })
  }

  setRotationsPerDay(rpd: any) {
    this.upload.rpd = rpd.value
    this.estimateDuration(this.upload.rpd);
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
      hour: this.selectedHour == null ? this.upload.hour : this.selectedHour,
      minutes: this.selectedMinutes == null ? this.upload.minutes : this.selectedMinutes,
    }

    this.apiService.updateState(ApiService.getWindowHref(window), body).subscribe((response) => {
      if (response.status == 204) {
        this.apiService.getStatus(ApiService.getWindowHref(window)).subscribe((data) => {
          this.upload.activityState = data.status;
          this.upload.rpd = data.rotationsPerDay;
          this.upload.direction = data.direction;
          this.upload.hour = data.hour;
          this.upload.minutes = data.minutes;
        })
      }
      this.upload.disabled = false;
      this.upload.statusMessage = 'Save Settings';
    });
  }

  beginProgramming(): void {
    this.uploadSettings('START');
  }

  stopProgramming(): void {
    this.uploadSettings('STOP');
  }

  // todo - double check math...
  estimateDuration(rpd: any) {
    const totalSecondsSpentTurning = rpd * this.upload.durationInSecondsToCompleteOneRevolution;
    const totalNumberOfRestingPeriods = totalSecondsSpentTurning / 180;
    const totalRestDuration = totalNumberOfRestingPeriods * 180;

    const finalRoutineDuration = totalRestDuration + totalSecondsSpentTurning;

    console.log(finalRoutineDuration);

    const readableDuration = new Date(0,0);
    readableDuration.setSeconds(finalRoutineDuration);
    console.log(readableDuration);
    
    // readableDuration.setMinutes(finalRoutineDuration / 60);
    console.log(readableDuration);


    const hours = readableDuration.toTimeString().slice(0,2);
    const mins = readableDuration.toTimeString().slice(3,5);

    this.estDuration = 
      hours < "10" ? `${hours.slice(1,2)} hours ${mins} minutes` :
    `${hours} hours ${mins} minutes`;
  }

}
