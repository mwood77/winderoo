import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs';
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

  static getWindowHref(_window: typeof window): string {
    // remove single trailing '/'
    const sanitizedHref =
      _window.location.href.substring(_window.location.href.length - 1) === '/'
        ? _window.location.href.substring(0, _window.location.href.length - 1)
        : _window.location.href;

    return sanitizedHref;
  }

  private constructURL(URL: string): string {
    return URL + "/api/";
  }

  getShouldRefresh() {
    return this.shouldRefresh$.asObservable();
  }

  getStatus(URL: string) {
    return this.http.get<Status>(this.constructURL(URL) + 'status');
  }

  updatePowerState(URL: string, powerState: boolean) {
    let powerStateToNum;
    const baseURL = this.constructURL(URL);
    
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

  updateTimerState(URL: string, timerState: boolean) {
    let timerStateToNum;
    const baseURL = this.constructURL(URL);
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

  updateState(URL: string, update: Update) {
    const baseURL = this.constructURL(URL);

    const constructedURL = baseURL
      + 'update?action=' + update.action + '&'
      + 'rotationDirection=' + update.rotationDirection + '&'
      + 'tpd=' + update.tpd + '&'
      + 'hour=' + update.hour + '&'
      + 'minutes=' + update.minutes;

    return this.http.post(constructedURL, null, { observe: 'response' });
  }

  resetDevice(URL: string) {
    return this.http.get<any>(this.constructURL(URL) + 'reset');
  }
}
