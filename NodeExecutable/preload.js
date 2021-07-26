// All of the Node.js APIs are available in the preload process.
// It has the same sandbox as a Chrome extension.
const {contextBridge,ipcRenderer} = require('electron')

const validChannels = ["openFileDialog","fileSelected","showPlugins","currentPluginListing","response"];

contextBridge.exposeInMainWorld(
  "api", {
    send: (channel, data) => {
        if (validChannels.includes(channel)) {
            ipcRenderer.send(channel, data);
        }
    },
    on: (channel, callback) => {
      if (validChannels.includes(channel)) {
        // Filtering the event param from ipcRenderer
        ipcRenderer.on(channel, callback);
      }
    },
  }
);


window.addEventListener('DOMContentLoaded', () => {
  const replaceText = (selector, text) => {
    const element = document.getElementById(selector)
    if (element) element.innerText = text
  }

})
