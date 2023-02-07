import { HttpClient } from '@angular/common/http';
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

  DEFUALT_URL = 'http://winderoo.local';

  constructor(private http: HttpClient) { }

  static getWindowHref(_window: typeof window): string {
      if (_window.location.href.includes('192')) {
        
        // remove single trailing '/' from ip
        const sanitizedHref = 
          _window.location.href.substring(_window.location.href.length - 1) === '/' ?
          _window.location.href.substring(0, _window.location.href.length - 1) :
          _window.location.href

      return sanitizedHref;
    }

    return 'http://winderoo.local';
  }

  private constructURL(URL: string): string {
    if (URL != this.DEFUALT_URL) {
      return URL + "/api/";
    }

    return this.DEFUALT_URL + '/api/';
  }

  getStatus(URL: string) {
    return this.http.get<Status>(this.constructURL(URL) + 'status');
  }

  updateState(URL: string, update: Update) {
    const baseURL = this.constructURL(URL);

    const constructedURL = baseURL
      + 'update?action=' + update.action + '&'
      + 'rotationDirection=' + update.rotationDirection +'&'
      + 'tpd=' + update.tpd +'&'
      + 'hour=' + update.hour +'&'
      + 'minutes=' + update.minutes;
    return this.http.post(constructedURL, null, { observe: 'response' });
  }

  resetDevice(URL: string) {
    return this.http.get<any>(this.constructURL(URL) + 'reset');
  }
}
