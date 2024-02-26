import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { environment } from '../environments/environment';
import { BehaviorSubject } from 'rxjs/internal/BehaviorSubject';


export interface Update {
  action?: string
  rotationDirection?: string,
  tpd?: number,
  hour?: string;
  minutes?: string;
  timerEnabled?: number;
}

export interface Status {
  batteryLevel: any;
  db: number;
  status: string;
  rotationsPerDay: number;
  direction: string;
  hour: string;
  minutes: string;
  durationInSecondsToCompleteOneRevolution: number;
  startTimeEpoch: number,
  currentTimeEpoch: number;
  estimatedRoutineFinishEpoch: number;
  winderEnabled: number;
  timerEnabled: number;
}

@Injectable({
  providedIn: 'root'
})
export class ApiService {

  isWinderEnabled$ = new BehaviorSubject(0);
  shouldRefresh$ = new BehaviorSubject(false);

  constructor(private http: HttpClient) { }

  static constructURL(): string {
    return environment.apiUrl + "/api/";
  }

  getShouldRefresh() {
    return this.shouldRefresh$.asObservable();
  }

  getStatus() {
    return this.http.get<Status>(ApiService.constructURL() + 'status');
  }

  updatePowerState(powerState: boolean) {
    let powerStateToNum;
    const baseURL = ApiService.constructURL();
    
    if (powerState) { 
      powerStateToNum = 1;
    } else {
      powerStateToNum = 0;
    }

    const constructedURL = baseURL 
      + "power?"
      + "winderEnabled=" + powerStateToNum;

    return this.http.post(constructedURL, null, { observe:'response' });
  }

  updateTimerState(timerState: boolean) {
    let timerStateToNum;
    const baseURL = ApiService.constructURL();
    console.log(timerState)
    if (timerState) {
      timerStateToNum = 1;
    } else {
      timerStateToNum = 0;
    }

    const constructedURL = baseURL
      + "update?"
      + "timerEnabled=" + timerStateToNum;

    return this.http.post(constructedURL, null, { observe: 'response' });
  }

  updateState(update: Update) {
    const baseURL = ApiService.constructURL();

    const constructedURL = baseURL
      + 'update?action=' + update.action + '&'
      + 'rotationDirection=' + update.rotationDirection + '&'
      + 'tpd=' + update.tpd + '&'
      + 'hour=' + update.hour + '&'
      + 'minutes=' + update.minutes;

    return this.http.post(constructedURL, null, { observe: 'response' });
  }

  resetDevice() {
    return this.http.get<any>(ApiService.constructURL() + 'reset');
  }
}
