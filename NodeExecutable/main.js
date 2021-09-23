// Modules to control application life and create native browser window
//This is the basic start point for the system
const {app, BrowserWindow,ipcMain} = require('electron')
const path = require('path')

function createWindow () {
		
  // Create the browser window.
  const mainWindow = new BrowserWindow({
    width: 800,
    height: 600,
    webPreferences: {
	  preload: path.join(__dirname, 'preload.js'), 
	  nodeIntegrationInWorker: true
	}
  })

  // and load the index.html of the app.
  mainWindow.loadFile('NodeExecutable/index.html')

  // Open the DevTools.
  // mainWindow.webContents.openDevTools()
}


ipcMain.on('openFileDialog', (event, arg) => {
	const buff = openFileDialogExplorer();
	event.reply('fileSelected',  buff);
})

ipcMain.on('showPlugins',(event,arg) =>
{
	const buff = showDefaultPlugins();
	event.reply('currentPluginListing');
})

ipcMain.on('openDirectorySelector',(event,arg) =>
{
	const buff = openDirectoryExplorer();
	event.reply('directorySelected',buff);
})

function openFileDialogExplorer()
{
	const { dialog } = require('electron')
	var returnValue = dialog.showOpenDialogSync({ properties: ['openFile','openDirectory','multiSelections'] });
	return returnValue;
}

function openDirectoryExplorer()
{
	const { dialog } = require('electron')
	var returnValue = new Object();
	 
	
	var directory = dialog.showOpenDialogSync({ 
		properties: ['openDirectory']});
	returnValue.selectedInputCodex = "DataExchangeFormat";
	returnValue.selectedOutputCodex = "DataExchangeFormat";
	returnValue.selectedDirectory = directory;
	return returnValue;
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.whenReady().then(() => {
  createWindow()
  
  app.on('activate', function () {
    // On macOS it's common to re-create a window in the app when the
    // dock icon is clicked and there are no other windows open.
    if (BrowserWindow.getAllWindows().length === 0) createWindow()
  })
})

// Quit when all windows are closed, except on macOS. There, it's common
// for applications and their menu bar to stay active until the user quits
// explicitly with Cmd + Q.
app.on('window-all-closed', function () {
  if (process.platform !== 'darwin') app.quit()
})

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.
