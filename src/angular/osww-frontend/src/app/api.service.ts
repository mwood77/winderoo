import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { environment } from '../environments/environment';
import { BehaviorSubject } from 'rxjs/internal/BehaviorSubject';


export interface Update {
  action: string
  rotationDirection: string,
  tpd: number,
  hour: string;
  minutes: string;
  timerEnabled: number;
  screenSleep: boolean;
  customWindDuration: number;
  customWindPauseDuration: number;
  customDurationInSecondsToCompleteOneRevolution: number;
  rtcSelectedHour: number;
  rtcSelectedMinutes: number;
}

export interface Status {
  batteryLevel: any;
  db: number;
  status: string;
  rotationsPerDay: number;
  direction: string;
  hour: string;
  minutes: string;
  startTimeEpoch: number,
  currentTimeEpoch: number;
  estimatedRoutineFinishEpoch: number;
  winderEnabled: number;
  timerEnabled: number;
  screenSleep: boolean;
  screenEquipped: boolean;
  customWindDuration: number;
  customWindPauseDuration: number;
  customDurationInSecondsToCompleteOneRevolution: number;
}

export interface WorldTimeAPI {
  utc_offset: string;
  timezone: string;
  day_of_week: number;
  day_of_year: number;
  datetime: string;
  utc_datetime: string;
  unixtime: number;
  raw_offset: number;
  week_number: number;
  dst: boolean;
  abbreviation: string;
  dst_offset: number;
  dst_from: string | null;
  dst_until: string | null;
  client_ip: string;
}

@Injectable({
  providedIn: 'root'
})
export class ApiService {

  isWinderEnabled$ = new BehaviorSubject(0);
  shouldRefresh$ = new BehaviorSubject(false);

  constructor(private http: HttpClient) { }

  static constructURL(): string {
    if (
      window.location.href.includes('127.0.0.1') ||
      window.location.href.includes('localhost')
    ) {
      return environment.apiUrl + "/api/";
    } else {
      // remove single trailing '/'
      const sanitizedHref =
        window.location.href.substring(window.location.href.length - 1) ===
        '/'
          ? window.location.href.substring(0, window.location.href.length - 1)
          : window.location.href;

      return sanitizedHref + "/api/";
    }
  }

  getShouldRefresh() {
    return this.shouldRefresh$.asObservable();
  }

  getStatus() {
    return this.http.get<Status>(ApiService.constructURL() + 'status');
  }

  updatePowerState(powerState: boolean) {
    let powerStateToNum;
    const baseURL = ApiService.constructURL() + 'power';

    if (powerState) {
      powerStateToNum = 1;
    } else {
      powerStateToNum = 0;
    }

    const powerBody = {
      winderEnabled: powerStateToNum
    }

    return this.http.post(baseURL, powerBody, { observe:'response' });
  }

  updateTimerState(timerState: number) {
    const baseURL = ApiService.constructURL();

    const constructedURL = baseURL
      + "timer?"
      + "timerEnabled=" + timerState;

    return this.http.post(constructedURL, null, { observe: 'response' });
  }

  updateState(update: Update) {
    const baseURL = ApiService.constructURL() + 'update';
    return this.http.post(baseURL, update, { observe: 'response' });
  }

  resetDevice() {
    return this.http.get<any>(ApiService.constructURL() + 'reset');
  }

}
