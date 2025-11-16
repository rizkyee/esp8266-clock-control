<template>
  <div
    class="bg-gray-800/60 backdrop-blur-md p-6 rounded-2xl shadow-xl w-full max-w-lg border border-gray-700"
  >
    <h2 class="text-lg font-semibold text-indigo-400 mb-3">Panel Kontrol</h2>

    <!-- 🔗 Input IP Perangkat -->
    <div class="flex items-center gap-2 mb-3">
      <input
        v-model="deviceIp"
        placeholder="IP perangkat (misal 192.168.1.123)"
        class="px-3 py-2 rounded-md bg-gray-900 border border-gray-700 text-white w-64"
      />
      <button
        @click="checkDevice"
        class="px-3 py-2 bg-indigo-600 hover:bg-indigo-700 rounded-md font-medium"
      >
        Check
      </button>
    </div>

    <div class="space-y-5">
      <!-- Tombol utama -->
      <div class="flex flex-wrap justify-center gap-3">
        <button
          @click="showTime"
          class="px-5 py-2.5 bg-indigo-500 hover:bg-indigo-600 rounded-lg font-semibold shadow-lg hover:shadow-indigo-500/40 transition-all"
        >
          Tampilkan Jam
        </button>

        <button
          @click="cekLingkungan"
          class="px-5 py-2.5 bg-green-500 hover:bg-green-600 rounded-lg font-semibold shadow-lg hover:shadow-green-500/40 transition-all"
        >
          Cek Suhu & Kelembapan
        </button>
      </div>

      <!-- 🔧 Atur Jam Manual -->
      <div class="border-t border-gray-700 pt-4 mt-4">
        <h3 class="text-md font-semibold text-indigo-300 mb-2">Atur Jam Manual</h3>

        <div class="flex flex-col gap-2">
          <input
            v-model="manualTime"
            type="time"
            class="bg-gray-900 border border-gray-700 px-3 py-2 rounded-md text-white"
          />
          <input
            v-model="manualDate"
            type="date"
            class="bg-gray-900 border border-gray-700 px-3 py-2 rounded-md text-white"
          />
          <select
            v-model="manualDay"
            class="bg-gray-900 border border-gray-700 px-3 py-2 rounded-md text-white"
          >
            <option v-for="day in daysEn" :key="day" :value="day">{{ day }}</option>
          </select>
        </div>

        <div class="flex justify-center gap-3 mt-3">
          <button
            @click="setManual"
            class="px-4 py-2 bg-blue-500 hover:bg-blue-600 rounded-lg font-semibold shadow-lg hover:shadow-blue-500/40 transition-all"
          >
            Set ke Alat
          </button>

          <button
            @click="syncAuto"
            class="px-4 py-2 bg-yellow-500 hover:bg-yellow-600 rounded-lg font-semibold shadow-lg hover:shadow-yellow-500/40 transition-all"
          >
            Cocokkan Otomatis
          </button>
        </div>
      </div>

      <!-- Input teks -->
      <div class="flex justify-center items-center gap-2 mt-5">
        <input
          v-model="text"
          type="text"
          placeholder="Tulis teks baru..."
          class="px-3 py-2 w-2/3 rounded-md bg-gray-900 border border-gray-700 text-white focus:outline-none"
        />
        <button
          @click="openModal"
          class="px-4 py-2 bg-blue-500 hover:bg-blue-600 rounded-lg font-semibold shadow-lg hover:shadow-blue-500/40 transition-all"
        >
          Kirim
        </button>
      </div>

      <!-- Kontrol tambahan -->
      <div class="mt-5 border-t border-gray-700 pt-4 space-y-3">
        <div>
          <label class="block text-gray-300 font-medium mb-1">Kecepatan teks:</label>
          <select
            v-model="speed"
            @change="changeSpeed"
            class="bg-gray-900 border border-gray-700 px-3 py-2 rounded-md w-full"
          >
            <option :value="25">Lambat</option>
            <option :value="18">Sedang</option>
            <option :value="10">Cepat</option>
          </select>
        </div>

        <div>
          <label class="block text-gray-300 font-medium mb-1">Arah teks:</label>
          <select
            v-model="direction"
            @change="changeDirection"
            class="bg-gray-900 border border-gray-700 px-3 py-2 rounded-md w-full"
          >
            <option value="left">Kiri</option>
            <option value="right">Kanan</option>
          </select>
        </div>

        <div>
          <label class="block text-gray-300 font-medium mb-1">Mode tampil:</label>
          <select
            v-model="mode"
            @change="changeMode"
            class="bg-gray-900 border border-gray-700 px-3 py-2 rounded-md w-full"
          >
            <option value="scroll">Berjalan</option>
            <option value="statis">Statis</option>
          </select>
        </div>
      </div>
    </div>

    <!-- Modal konfirmasi & Toast -->
    <transition name="fade">
      <teleport to="body">
        <div
          v-if="showConfirmModal"
          class="fixed inset-0 flex items-center justify-center bg-black/70 backdrop-blur-sm z-[9999]"
        >
          <div
            class="relative bg-gray-900/95 rounded-2xl p-8 w-[400px] text-center shadow-2xl border border-indigo-500/40 animate-zoomIn"
          >
            <button
              @click="closeModal"
              class="absolute top-2 right-3 text-gray-400 hover:text-white text-2xl font-bold transition"
            >
              ×
            </button>

            <h3 class="text-xl font-semibold text-indigo-400 mb-3">
              Konfirmasi Tindakan
            </h3>
            <p class="text-gray-300 mb-6 text-sm leading-relaxed">
              Apakah kamu ingin menampilkan teks di website saja atau mengirimkannya ke alat?
            </p>

            <div class="flex justify-center gap-3">
              <button
                @click="previewWeb"
                class="px-4 py-2 bg-blue-500 hover:bg-blue-600 rounded-lg font-semibold shadow-lg hover:shadow-blue-500/40 transition-all"
              >
                Preview di Web
              </button>
              <button
                @click="sendToDevice"
                class="px-4 py-2 bg-green-500 hover:bg-green-600 rounded-lg font-semibold shadow-lg hover:shadow-green-500/40 transition-all"
              >
                Kirim ke Alat
              </button>
            </div>

            <button
              @click="closeModal"
              class="mt-6 text-sm text-gray-400 hover:text-gray-200 underline transition"
              >Batal</button
            >
          </div>
        </div>
      </teleport>
    </transition>

    <!-- Toast -->
    <teleport to="body">
      <transition name="fade">
        <div
          v-if="toastMessage"
          :class="[
            'fixed bottom-6 right-6 flex items-center gap-3 px-4 py-3 rounded-lg shadow-lg border backdrop-blur-md min-w-[250px] z-[99999]',
            toastType === 'success'
              ? 'bg-green-600/80 border-green-400 text-white'
              : 'bg-red-600/80 border-red-400 text-white',
          ]"
        >
          <span class="font-medium">{{ toastMessage }}</span>
        </div>
      </transition>
    </teleport>
  </div>
</template>

<script setup>
import { ref, onMounted, watch } from 'vue'

const text = ref('')
const temp = ref('')
const humid = ref('')
const speed = ref(18)
const direction = ref('left')
const mode = ref('scroll')
const deviceIp = ref(localStorage.getItem('deviceIp') || '')
const showConfirmModal = ref(false)

const toastMessage = ref(null)
const toastType = ref('success')
let toastTimeout = null

// Hari (English)
const daysEn = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday']

// Input manual waktu
const manualTime = ref('')
const manualDate = ref('')
const manualDay = ref('Monday')

const emit = defineEmits([
  'updateDisplay',
  'updateSpeed',
  'updateDirection',
  'updateMode',
  'sendToDevice',
  'showTime',
  'setManualTime',
  'syncAutoTime',
])

// 🔗 Simpan IP di localStorage
watch(deviceIp, (val) => localStorage.setItem('deviceIp', val))

// ✅ Check perangkat
const checkDevice = async () => {
  if (!deviceIp.value) return showToast('Isi IP perangkat dulu', 'error')
  try {
    const controller = new AbortController()
    const timeoutId = setTimeout(() => controller.abort(), 3000) // timeout 3 detik
    const res = await fetch(`http://${deviceIp.value}/status`, {
      mode: 'cors', // tambahkan ini
      signal: controller.signal
    })
    clearTimeout(timeoutId)
    if (!res.ok) throw new Error(`Status code ${res.status}`)
    const data = await res.json()
    showToast('Perangkat online: ' + data.time, 'success')
  } catch (err) {
    console.error(err)
    showToast('Gagal terhubung ke perangkat', 'error')
  }
}

// Auto refresh setiap 5 detik
setInterval(async () => {
  if (!deviceIp.value) return
  try {
    const res = await fetch(`http://${deviceIp.value}/status`)
    if (!res.ok) return
    const data = await res.json()
    emit('updateDisplay', {
      temp: data.temperature,
      humid: data.humidity,
    })
  } catch (err) {
    console.log('Device not reachable')
  }
}, 5000)


// 🕒 Atur waktu manual
const setManual = async () => {
  if (!manualTime.value || !manualDate.value) return
  emit('setManualTime', {
    time: manualTime.value,
    date: manualDate.value,
    day: manualDay.value,
  })
  showToast('Jam manual berhasil diatur!', 'success')

  if (deviceIp.value) {
    try {
      const timeFull =
        manualTime.value.length === 5 ? `${manualTime.value}:00` : manualTime.value
      const url = `http://${deviceIp.value}/set-time?time=${encodeURIComponent(
        timeFull
      )}&date=${encodeURIComponent(manualDate.value)}&day=${encodeURIComponent(
        manualDay.value
      )}`
      const resp = await fetch(url)
      if (resp.ok) showToast('Terkirim ke perangkat', 'success')
      else showToast('Perangkat menolak permintaan', 'error')
    } catch {
      showToast('Gagal kirim ke perangkat', 'error')
    }
  }
}

// 🔄 Sinkron otomatis
const syncAuto = async () => {
  emit('syncAutoTime')
  showToast('Jam disinkron otomatis!', 'success')

  if (!deviceIp.value) return
  try {
    const resp = await fetch(`http://${deviceIp.value}/sync`)
    if (resp.ok) showToast('Perangkat sinkron NTP', 'success')
    else showToast('Perangkat gagal sinkron', 'error')
  } catch {
    showToast('Gagal konek ke perangkat', 'error')
  }
}

// 📨 Kirim teks ke ESP
const sendToDevice = async () => {
  if (!text.value) return
  emit('sendToDevice', {
    text: text.value,
    speed: speed.value,
    direction: direction.value,
    mode: mode.value,
  })
  closeModal()
  if (!deviceIp.value) return showToast('IP perangkat kosong', 'error')
  try {
    const url = `http://${deviceIp.value}/set-text?text=${encodeURIComponent(text.value)}`
    const r = await fetch(url)
    if (r.ok) showToast('Teks terkirim ke perangkat', 'success')
    else showToast('Perangkat menolak teks', 'error')
  } catch {
    showToast('Gagal kirim teks ke perangkat', 'error')
  }
  text.value = ''
}

// Fitur lainnya
const showTime = () => emit('showTime')
const cekLingkungan = () => {
  temp.value = (25 + Math.random() * 5).toFixed(1)
  humid.value = (60 + Math.random() * 10).toFixed(1)
  emit('updateDisplay', { temp: temp.value, humid: humid.value })
}

// Modal
const openModal = () => {
  if (text.value.trim() !== '') showConfirmModal.value = true
}
const closeModal = () => (showConfirmModal.value = false)

const previewWeb = () => {
  emit('updateDisplay', { text: text.value })
  closeModal()
  showToast('Preview di Web berhasil!', 'success')
  text.value = ''
}

// Toast
const showToast = (msg, type = 'success') => {
  if (toastTimeout) clearTimeout(toastTimeout)
  toastMessage.value = msg
  toastType.value = type
  toastTimeout = setTimeout(() => (toastMessage.value = null), 3000)
}

// Simpan preferensi
onMounted(() => {
  const s = localStorage.getItem('speed')
  const d = localStorage.getItem('direction')
  const m = localStorage.getItem('mode')
  if (s) speed.value = Number(s)
  if (d) direction.value = d
  if (m) mode.value = m
})
watch(speed, (v) => localStorage.setItem('speed', v))
watch(direction, (v) => localStorage.setItem('direction', v))
watch(mode, (v) => localStorage.setItem('mode', v))

const changeSpeed = () => emit('updateSpeed', speed.value)
const changeDirection = () => emit('updateDirection', direction.value)
const changeMode = () => emit('updateMode', mode.value)
</script>

<style scoped>
.fade-enter-active,
.fade-leave-active {
  transition: opacity 0.5s, transform 0.5s;
}
.fade-enter-from,
.fade-leave-to {
  opacity: 0;
  transform: translateY(20px);
}
</style>
