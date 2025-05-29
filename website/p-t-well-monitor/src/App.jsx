import { useState } from 'react'
import reactLogo from './assets/react.svg'
import viteLogo from '/vite.svg'
import './App.css'
import AppNotFound from './pages/AppNotFound/AppNotFound.jsx'
import {Routes, Route} from 'react-router-dom'

function App() {
  const [count, setCount] = useState(0)

  return(
    <>
    <Routes>
      <Route path="*" element={<AppNotFound />} />
    </Routes>
    </>
  )
}

export default App
