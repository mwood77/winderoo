import { Component, OnInit } from '@angular/core';
import { ApiService, Update } from '../api.service';
import { ProgressBarMode } from '@angular/material/progress-bar';

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
      { value: '23', viewValue: '23' },
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
    durationInSecondsToCompleteOneRevolution: 0,
    startTimeEpoch: 0,
    estimatedRoutineFinishEpoch: 0,
    isTimerEnabledNum: 0
  }

  selectedHour: any;
  selectedMinutes: any;
  estDuration: any;

  progressMode: ProgressBarMode = 'indeterminate';
  progressPercentageComplete: number =  0;
  isWinderEnabled: number;
  isTimerEnabled: boolean;

  watchWindingParametersURL = 'https://watch-winder.store/watch-winding-table/';

  constructor(private apiService: ApiService) {
    this.isWinderEnabled = this.apiService.isWinderEnabled$.getValue();
    this.isTimerEnabled = false;
   }

  ngOnInit(): void {
    this.upload.statusMessage = 'Save Settings'
    this.getData();
    this.setupSubscriptions();
  }

  setupSubscriptions(): void {
    this.apiService.isWinderEnabled$.subscribe((e) => {
      this.isWinderEnabled = e;
    })

    // Retrieve updated status data on power toggle change
    this.apiService.getShouldRefresh().subscribe((result) => {
      if (result === true) {
        this.apiService.shouldRefresh$.next(false);
        this.getData();
      }
    })
  }

  getData(): void {
    this.apiService.getStatus(ApiService.getWindowHref(window)).subscribe((data) => {
      this.upload.activityState = data.status;
      this.upload.rpd = data.rotationsPerDay;
      this.upload.direction = data.direction;
      this.upload.hour = data.hour;
      this.upload.minutes = data.minutes;
      this.wifiSignalIcon = this.getWifiSignalStrengthIcon(data.db *= -1);
      this.upload.durationInSecondsToCompleteOneRevolution = data.durationInSecondsToCompleteOneRevolution;
      this.upload.startTimeEpoch = data.startTimeEpoch;
      this.upload.estimatedRoutineFinishEpoch = data.estimatedRoutineFinishEpoch;
      this.upload.isTimerEnabledNum = data.timerEnabled;

      this.apiService.isWinderEnabled$.next(data.winderEnabled);

      this.mapTimerEnabledState(this.upload.isTimerEnabledNum);
      this.estimateDuration(this.upload.rpd);
      this.getProgressComplete(data.startTimeEpoch, data.currentTimeEpoch, data.estimatedRoutineFinishEpoch);
    });
  }

  setRotationsPerDay(rpd: any): void {
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

  getWifiSignalStrengthIcon(db: number): string {
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

  openSite(URL: string): void {
    window.open(URL, '_blank');
  }

  uploadSettings(actionToDo?: string): void {
    this.upload.statusMessage = 'Upload in progress';
    this.upload.disabled = true;

    const body: Update = {
      action: actionToDo ? actionToDo : this.getActivityStateForAPI(this.upload.activityState),
      rotationDirection: this.upload.direction,
      tpd: this.upload.rpd,
      hour: this.selectedHour == null ? this.upload.hour : this.selectedHour,
      minutes: this.selectedMinutes == null ? this.upload.minutes : this.selectedMinutes,
      timerEnabled: this.upload.isTimerEnabledNum,
    }

    this.apiService.updateState(ApiService.getWindowHref(window), body).subscribe((response) => {
      if (response.status == 204) {
        this.getData();
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

  mapTimerEnabledState($event: number): void {
    if ($event == 1) {
      this.isTimerEnabled = true;
    } else {
      this.isTimerEnabled = false;
    };
  }

  estimateDuration(rpd: number): void {
    const totalSecondsSpentTurning = rpd * this.upload.durationInSecondsToCompleteOneRevolution;
    const totalNumberOfRestingPeriods = totalSecondsSpentTurning / 180;
    const totalRestDuration = totalNumberOfRestingPeriods * 180;

    const finalRoutineDuration = totalRestDuration + totalSecondsSpentTurning;

    const readableDuration = new Date(0,0);
    readableDuration.setSeconds(finalRoutineDuration);
    const hours = readableDuration.toTimeString().slice(0,2);
    const mins = readableDuration.toTimeString().slice(3,5);

    this.estDuration = 
      hours < "10" ? `${hours.slice(1,2)} hours ${mins} minutes` :
      `${hours} hours ${mins} minutes`;
  }

  getProgressComplete(startTimeEpoch: number, currentTimeEpoch: number, estimatedRoutineFinishEpoch: number): void {
    if (this.upload.activityState == 'Winding' && currentTimeEpoch < estimatedRoutineFinishEpoch) {
      this.progressMode = 'determinate';
      const difference = (currentTimeEpoch - startTimeEpoch) / (estimatedRoutineFinishEpoch - startTimeEpoch);
      const percentage = difference * 100;

      // When 'Start" button pressed
      if (percentage <= 0.05) {
        this.progressMode = 'indeterminate';
        setTimeout(() => this.getData(), 5000);
      }

      this.progressPercentageComplete = percentage;
    }
  }

  updateTimerEnabledState($state: any) {
    this.upload.isTimerEnabledNum = $state;
    this.apiService.updateTimerState(ApiService.getWindowHref(window), $state).subscribe(
      (data) => {
        this.mapTimerEnabledState($state)
      });
  };

}
