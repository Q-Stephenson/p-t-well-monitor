import { useState } from 'react'
import reactLogo from './assets/react.svg'
import viteLogo from '/vite.svg'
import './App.css'

function App() {
  const [connected, setConnected] = useState(false)

  const connectSerial = async () => {
    try {
      const port = await navigator.serial.requestPort(
        { filters: [{ usbVendorId: 0x2e8a }] }
      )
      await port.open({ baudRate: 115200 })
      setConnected(true)
    } catch (error) {
      console.error('Error connecting to serial port:', error)
    }
  }

  const disconnectSerial = async () => {
    try {
      const port = await navigator.serial.requestPort()
      await port.close()
      setConnected(false)
    } catch (error) {
      console.error('Error disconnecting from serial port:', error)
    }
  }

  return (
    <div>
      <h1>Serial Port Controller</h1>
      <button onClick={connected ? disconnectSerial : connectSerial}>
        {connected ? 'Disconnect' : 'Connect'} to Serial Port
      </button>
    </div>
  )
}

export default App
