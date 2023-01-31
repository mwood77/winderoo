import { Component, OnInit } from '@angular/core';
import { MatDialog, MatDialogRef } from '@angular/material/dialog';
import { ApiService } from '../api.service';

@Component({
  selector: 'app-header',
  templateUrl: './header.component.html',
  styleUrls: ['./header.component.scss']
})
export class HeaderComponent implements OnInit {

  resetting = false;

  constructor(private apiService: ApiService, private dialog: MatDialog) { }

  ngOnInit(): void {}

  openDialog(): void {
    this.dialog.open(DialogAnimationsExampleDialog, {
      width: '80%',
    });
  }

  reset() {
    this.resetting = true;
    this.openDialog();
    this.apiService.resetDevice().subscribe();
  }

}

@Component({
  selector: 'app-header-dialog',
  templateUrl: './header-dialog.component.html',
})
export class DialogAnimationsExampleDialog {
  constructor(public dialogRef: MatDialogRef<DialogAnimationsExampleDialog>) {}
}