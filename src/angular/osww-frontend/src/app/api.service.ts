import { HttpClient, HttpParams } from '@angular/common/http';
import { Injectable } from '@angular/core';


export interface Update {
  action: string
  rotationDirection: string,
  tpd: number,
  hour: string;
  minutes: string;
}

export interface Status {
  batteryLevel: any;
  db: number;
  status: string;
  rotationsPerDay: number;
  direction: string;
  hour: string;
  minutes: string;
}

@Injectable({
  providedIn: 'root'
})
export class ApiService {

  URL = 'http://winderoo.local/api/';

  constructor(private http: HttpClient) { }

  getStatus() {
    return this.http.get<Status>(this.URL + 'status');
  }

  updateState(update: Update) {
    const constructedURL = this.URL 
      + 'update?action=' + update.action + '&'
      + 'rotationDirection=' + update.rotationDirection +'&'
      + 'tpd=' + update.tpd +'&'
      + 'hour=' + update.hour +'&'
      + 'minutes=' + update.minutes;
    return this.http.post(constructedURL, null, { observe: 'response' });
  }

  resetDevice() {
    return this.http.get<any>(this.URL + 'reset');
  }
}
