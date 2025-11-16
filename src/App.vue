<template>
  <div class="flex flex-col min-h-screen">
    <HeaderBar />

    <main class="flex-grow flex flex-col items-center justify-center text-center p-8">
      <DisplayStatus :displayText="displayText" :temperature="temperature" :humidity="humidity" :deviceIp="deviceIp" />

      <ControlPanel @updateDisplay="updateDisplay" @updateSpeed="updateSpeed" @updateDirection="updateDirection"
        @updateMode="updateMode" @showTime="showTemporaryJam" @setManualTime="setManualTime"
        @syncAutoTime="syncAutoTime" @deviceConnected="handleDeviceConnected" />
    </main>

    <footer class="text-gray-400 text-sm py-4 text-center border-t border-gray-700">
      © 2025 Jam Digital | by Rizkyee
    </footer>
  </div>
</template>

<script setup>
import HeaderBar from './components/HeaderBar.vue'
import DisplayStatus from './components/DisplayStatus.vue'
import ControlPanel from './components/ControlPanel.vue'
import { ref, onMounted } from 'vue'

const displayText = ref('Menunggu data dari alat...')
const temperature = ref(null)
const humidity = ref(null)
const deviceIp = ref(localStorage.getItem('deviceIp') || '')

async function ambilDataDariAlat() {
  if (!deviceIp.value) {  
    displayText.value = 'IP perangkat belum diisi'
    return
  }

  try {
    const controller = new AbortController()
    const timeout = setTimeout(() => controller.abort(), 3000)
    const res = await fetch(`http://${deviceIp.value}/status`, {
      signal: controller.signal
    })
    clearTimeout(timeout)

    if (!res.ok) throw new Error('Gagal ambil data')

    const data = await res.json()
    temperature.value = data.temperature ?? '-'
    humidity.value = data.humidity ?? '-'

    // kalau alat kirim suhu dan kelembapan, tampilkan lengkap
    if (data.temperature !== undefined && data.humidity !== undefined) {
      displayText.value = `${data.time} | T:${data.temperature}°C | H:${data.humidity}%`
    } else {
      // kalau tidak, tampil jam saja
      displayText.value = data.time.length > 5 ? data.time.slice(0, 5) : data.time
    }
  } catch (err) {
    displayText.value = 'Tidak bisa terhubung ke perangkat'
  }
}

// Dipanggil saat tombol “Check IP” ditekan
function handleDeviceConnected(ip, success) {
  deviceIp.value = ip
  localStorage.setItem('deviceIp', ip)

  if (success) {
    ambilDataDariAlat()
  } else {
    displayText.value = 'Tidak bisa terhubung ke perangkat'
  }
}

// function setManualTime({ time }) {
//   displayText.value = `Jam diatur manual ke ${time}`
// }


onMounted(() => {
  ambilDataDariAlat()
  setInterval(ambilDataDariAlat, 5000)
})

// Ketika “Tampilkan Jam” diklik
function showTemporaryJam() {
  if (!deviceIp.value) {
    displayText.value = 'IP perangkat belum diisi'
    return
  }

  fetch(`http://${deviceIp.value}/status`)
    .then((r) => r.json())
    .then((data) => {
      // tampil hanya jam dan menit, tanpa suhu & kelembapan
      const jamHanya = data.time?.slice(0, 5) || '00:00'
      displayText.value = jamHanya
    })
    .catch(() => {
      displayText.value = 'Tidak bisa menampilkan jam'
    })
}

// function setManualTime({ time, date, day }) {
//   displayText.value = `${time} | ${day}, ${date}`
// }

function setManualTime({ time, date, day }) {
  if (date && day) {
    displayText.value = `${time} | ${day}, ${date}`
  } else {
    displayText.value = `Jam diatur manual ke ${time}`
  }
}


function syncAutoTime() {
  if (!deviceIp.value) return
  ambilDataDariAlat()
}

function updateDisplay(data) {
  if (data.text) displayText.value = data.text
}

function updateSpeed() {}
function updateDirection() {}
function updateMode() {}
</script>

