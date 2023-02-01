import { Component, OnInit } from '@angular/core';
import { MatDialog, MatDialogRef } from '@angular/material/dialog';
import { ApiService } from '../api.service';

@Component({
  selector: 'app-header',
  templateUrl: './header.component.html',
  styleUrls: ['./header.component.scss']
})
export class HeaderComponent implements OnInit {



  constructor(private dialog: MatDialog) { }

  ngOnInit(): void {}

  openDialog(): void {
    this.dialog.open(DialogAnimationsExampleDialog, {
      width: '80%',
    });
  }

  reset() {
    this.openDialog();
  }
  
}

@Component({
  selector: 'app-header-dialog',
  templateUrl: './header-dialog.component.html',
  styleUrls: ['./header-dialog.component.scss']
})
export class DialogAnimationsExampleDialog {

  constructor(public dialogRef: MatDialogRef<DialogAnimationsExampleDialog>, 
    private apiService: ApiService) {}
  
  confirmReset() {
    this.apiService.resetDevice().subscribe();
  }

}