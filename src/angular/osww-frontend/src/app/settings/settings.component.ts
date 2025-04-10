import { Component, OnInit, AfterViewChecked } from '@angular/core';
import { ApiService, Update } from '../api.service';
import { ProgressBarMode } from '@angular/material/progress-bar';
import { TranslateService } from '@ngx-translate/core';
import { ClockService } from '../clock.service';

interface SelectInterface {
  value: string;
  viewValue: string;
}

@Component({
  selector: 'app-settings',
  templateUrl: './settings.component.html',
  styleUrls: ['./settings.component.scss']
})
export class SettingsComponent implements OnInit, AfterViewChecked {

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

  gmtOffsets: SelectInterface[] = [
    { value: '-12', viewValue: 'UTC-12:00' },
    { value: '-11', viewValue: 'UTC-11:00' },
    { value: '-10', viewValue: 'UTC-10:00' },
    { value: '-9.5', viewValue: 'UTC-09:30' },
    { value: '-9', viewValue: 'UTC-09:00' },
    { value: '-8', viewValue: 'UTC-08:00' },
    { value: '-7', viewValue: 'UTC-07:00' },
    { value: '-6', viewValue: 'UTC-06:00' },
    { value: '-5', viewValue: 'UTC-05:00' },
    { value: '-4.5', viewValue: 'UTC-04:30' },
    { value: '-4', viewValue: 'UTC-04:00' },
    { value: '-3.5', viewValue: 'UTC-03:30' },
    { value: '-3', viewValue: 'UTC-03:00' },
    { value: '-2', viewValue: 'UTC-02:00' },
    { value: '-1', viewValue: 'UTC-01:00' },
    { value: '0', viewValue: 'UTC±00:00' },
    { value: '1', viewValue: 'UTC+01:00' },
    { value: '2', viewValue: 'UTC+02:00' },
    { value: '3', viewValue: 'UTC+03:00' },
    { value: '3.5', viewValue: 'UTC+03:30' },
    { value: '4', viewValue: 'UTC+04:00' },
    { value: '4.5', viewValue: 'UTC+04:30' },
    { value: '5', viewValue: 'UTC+05:00' },
    { value: '5.5', viewValue: 'UTC+05:30' },
    { value: '5.75', viewValue: 'UTC+05:45' },
    { value: '6', viewValue: 'UTC+06:00' },
    { value: '6.5', viewValue: 'UTC+06:30' },
    { value: '7', viewValue: 'UTC+07:00' },
    { value: '8', viewValue: 'UTC+08:00' },
    { value: '8.75', viewValue: 'UTC+08:45' },
    { value: '9', viewValue: 'UTC+09:00' },
    { value: '9.5', viewValue: 'UTC+09:30' },
    { value: '10', viewValue: 'UTC+10:00' },
    { value: '10.5', viewValue: 'UTC+10:30' },
    { value: '11', viewValue: 'UTC+11:00' },
    { value: '11.5', viewValue: 'UTC+11:30' },
    { value: '12', viewValue: 'UTC+12:00' },
    { value: '12.75', viewValue: 'UTC+12:45' },
    { value: '13', viewValue: 'UTC+13:00' },
    { value: '14', viewValue: 'UTC+14:00' }
  ];


  // populated by ngOnInit
  rtc_minutes: SelectInterface[] = [];

  wifiSignalIcon = ''

  upload = {
    activityState: '',
    statusMessage: '',
    disabled: false,
    direction: '',
    rpd: 0,
    hour: '00',
    minutes: '00',
    startTimeEpoch: 0,
    estimatedRoutineFinishEpoch: 0,
    isTimerEnabledNum: 0,
    screenSleep: false,
    customWindDuration: 0,
    customWindPauseDuration: 0,
    customDurationInSecondsToCompleteOneRevolution: 0,
    gmtOffset: 0,
    dst: false,
  }

  selectedHour: any;
  selectedMinutes: any;
  estHoursDuration: string = "";
  estMinutesDuration: string = "";
  
  winderooInternalRTC: number = 0;
  refreshingRTC: boolean = false;
  rtcGmtOffsets: any;

  progressMode: ProgressBarMode = 'indeterminate';
  progressPercentageComplete: number =  0;
  isWinderEnabled: number;
  isTimerEnabled: boolean;
  screenEquipped: boolean = false;

  watchWindingParametersURL = 'https://watch-winder.store/watch-winding-table/';

  screenScheduleEnabled: boolean = false;
  screenScheduleStartTime: { hour: string, minute: string } = { hour: '00', minute: '00' };
  screenScheduleEndTime: { hour: string, minute: string } = { hour: '00', minute: '00' };

  constructor(private apiService: ApiService, private translateService: TranslateService, private clockService: ClockService) {
    this.isWinderEnabled = this.apiService.isWinderEnabled$.getValue();
    this.isTimerEnabled = false;
  }
  ngAfterViewChecked(): void {
    this.upload.statusMessage = this.translateService.instant('SETTINGS.SAVE');
  }

  ngOnInit(): void {
    this.getData();
    this.setupSubscriptions();

    // Populate RTC minutes array
    for (let i = 0; i < 60; i++) {
      this.rtc_minutes.push(
        { value: i.toString(), viewValue: i.toString() },
      )
    }
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
    this.refreshingRTC = true
    this.clockService.stopClock();

    this.apiService.getStatus().subscribe((data) => {
      this.upload.activityState = data.status;
      this.upload.rpd = data.rotationsPerDay;
      this.upload.direction = data.direction;
      this.upload.hour = data.hour;
      this.upload.minutes = data.minutes;
      this.wifiSignalIcon = this.getWifiSignalStrengthIcon(data.db *= -1);
      this.upload.startTimeEpoch = data.startTimeEpoch;
      this.upload.estimatedRoutineFinishEpoch = data.estimatedRoutineFinishEpoch;
      this.upload.isTimerEnabledNum = data.timerEnabled;
      this.upload.screenSleep = data.screenSleep;
      this.screenEquipped = data.screenEquipped;
      this.winderooInternalRTC = data.currentTimeEpoch;
      this.upload.customDurationInSecondsToCompleteOneRevolution = data.customDurationInSecondsToCompleteOneRevolution;
      this.upload.gmtOffset = data.gmtOffset;
      this.upload.dst = data.dst;

      if (data.customWindDuration) {
        this.upload.customWindDuration = data.customWindDuration;
      } else {
        this.upload.customWindDuration = 180;
      }

      if (data.customWindPauseDuration) {
        this.upload.customWindPauseDuration = data.customWindPauseDuration;
      } else {
        this.upload.customWindPauseDuration = 15;
      }

      this.apiService.isWinderEnabled$.next(data.winderEnabled);

      this.mapTimerEnabledState(this.upload.isTimerEnabledNum);
      this.estimateDuration(this.upload.rpd);
      this.getProgressComplete(data.startTimeEpoch, data.currentTimeEpoch, data.estimatedRoutineFinishEpoch);

      this.clockService.startClock(this.winderooInternalRTC, (updatedEpoch: number) => {
        this.winderooInternalRTC = updatedEpoch * 1000; // Convert seconds to milliseconds for display
      });

      // Reset the RTC dropdowns
      this.rtcGmtOffsets = this.upload.gmtOffset;

      setTimeout(() => {
        this.refreshingRTC = false;
      }, 1500)
    });
  }

  setRotationsPerDay(rpd: any): void {
    this.upload.rpd = rpd.value
    this.estimateDuration(this.upload.rpd);
  }

  setCustomWindDuration(event: any): void {
    const seconds = event.value;
    this.upload.customWindDuration = seconds;
    this.estimateDuration(this.upload.rpd);
  }
  
  setCustomWindPauseDuration(event: any): void {
    const seconds = event.value;
    this.upload.customWindPauseDuration = seconds;
    this.estimateDuration(this.upload.rpd);
  }

  setcustomDurationInSecondsToCompleteOneRevolution(event: any): void {
    const customSingleRotationTime = event.value;
    this.upload.customDurationInSecondsToCompleteOneRevolution = customSingleRotationTime;
  }

  convertSecondsToHumanReadable(seconds: number): { minutes: number, seconds: number } {
    const minutes = Math.floor(seconds / 60);
    const remainingSeconds = seconds % 60;

    return {
      minutes,
      seconds: remainingSeconds,
    }
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
        return this.translateService.instant('SETTINGS.CLOCKWISE');
      case 'CCW':
        return this.translateService.instant('SETTINGS.COUNTER_CLOCKWISE');
      case 'BOTH':
        return this.translateService.instant('SETTINGS.BOTH');
      default:
        return this.translateService.instant('SETTINGS.CLOCKWISE');
    }
  }

  getReadableActivityState(activityState: string): string {
    switch (activityState) {
      case 'Winding':
        return this.translateService.instant('SETTINGS.WINDING');
      case 'Stopped':
        return this.translateService.instant('SETTINGS.STOPPED');
      default:
        return this.translateService.instant('SETTINGS.STOPPED');
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
    this.upload.statusMessage = this.translateService.instant('SETTINGS.SAVE_IN_PROGRESS');
    this.upload.disabled = true;

    const body: Update = {
      action: actionToDo ? actionToDo : this.getActivityStateForAPI(this.upload.activityState),
      rotationDirection: this.upload.direction,
      tpd: this.upload.rpd,
      hour: this.selectedHour == null ? this.upload.hour : this.selectedHour,
      minutes: this.selectedMinutes == null ? this.upload.minutes : this.selectedMinutes,
      timerEnabled: this.upload.isTimerEnabledNum,
      screenSleep: this.upload.screenSleep,
      screenScheduleEnabled: this.screenScheduleEnabled,
      screenScheduleStartTime: `${this.screenScheduleStartTime.hour}:${this.screenScheduleStartTime.minute}`,
      screenScheduleEndTime: `${this.screenScheduleEndTime.hour}:${this.screenScheduleEndTime.minute}`,
      customWindDuration: this.upload.customWindDuration,
      customWindPauseDuration: this.upload.customWindPauseDuration,
      customDurationInSecondsToCompleteOneRevolution: this.upload.customDurationInSecondsToCompleteOneRevolution,
      rtcGmtOffset: this.rtcGmtOffsets,
      rtcDST: this.upload.dst,
    }

    this.apiService.updateState(body).subscribe((response) => {
      if (response.status == 204) {
        this.getData();
      }

      this.upload.disabled = false;
      this.upload.statusMessage = this.translateService.instant('SETTINGS.SAVE');;
    });
  }

  beginProgramming(): void {
    this.uploadSettings('START');
  }

  stopProgramming(): void {
    this.uploadSettings('STOP');
  }

  mapTimerEnabledState(enabledState: number): void {
    if (enabledState == 1) {
      this.isTimerEnabled = true;
    } else {
      this.isTimerEnabled = false;
    };
  }

  estimateDuration(rpd: number): void {
    const totalSecondsSpentTurning = rpd * this.upload.customDurationInSecondsToCompleteOneRevolution;
    const totalNumberOfRestingPeriods = totalSecondsSpentTurning / this.upload.customWindDuration;
    const totalRestDuration = totalNumberOfRestingPeriods * this.upload.customWindPauseDuration;

    const finalRoutineDuration = totalRestDuration + totalSecondsSpentTurning;

    const readableDuration = new Date(0,0);
    readableDuration.setSeconds(finalRoutineDuration);
    const hours = readableDuration.toTimeString().slice(0,2);
    const mins = readableDuration.toTimeString().slice(3,5);

    this.estHoursDuration = hours < "10" ? hours.slice(1,2) : hours;
    this.estMinutesDuration = mins;
  }

  getProgressComplete(startTimeEpoch: number, currentTimeEpoch: number, estimatedRoutineFinishEpoch: number): void {
    if (this.upload.activityState == 'Winding' && currentTimeEpoch < estimatedRoutineFinishEpoch) {
      this.progressMode = 'determinate';
      const difference = (currentTimeEpoch - startTimeEpoch) / (estimatedRoutineFinishEpoch - startTimeEpoch);
      const percentage = difference * 100;

      // When "Start" button pressed
      if (percentage <= 0.05) {
        this.progressMode = 'indeterminate';
        setTimeout(() => this.getData(), 2500);
      }

      // Add 2 percent to make the progress bar look more full at lower percentages
      if (percentage < 10)  {
        this.progressPercentageComplete = percentage + 2;
      } else {
        this.progressPercentageComplete = percentage;
      }
    }
  }

  updateTimerEnabledState($state: boolean) {
    let timerStateToNum;
    if ($state) {
      timerStateToNum = 1;
    } else {
      timerStateToNum = 0;
    }
    this.upload.isTimerEnabledNum = timerStateToNum;
    this.apiService.updateTimerState(this.upload.isTimerEnabledNum).subscribe(
      (response) => {
        if (response.status == 204) {
          this.mapTimerEnabledState(this.upload.isTimerEnabledNum)
        }
      });
  };

  updateScreenSleepState($state: boolean) {
    this.upload.screenSleep = $state;
    this.uploadSettings();
  };

  updateDstState($state: boolean) {
    this.upload.dst = $state;
    this.uploadSettings();
  }

  updateScreenScheduleEnabled($state: boolean) {
    this.screenScheduleEnabled = $state;
    this.uploadSettings();
  }

  updateScreenScheduleTime(type: 'start' | 'end', time: { hour: string, minute: string }) {
    if (type === 'start') {
      this.screenScheduleStartTime = time;
    } else {
      this.screenScheduleEndTime = time;
    }
    this.uploadSettings();
  }

}
