<template>
  <div class="flex flex-col items-center w-full max-w-lg mx-auto p-6 space-y-6">

    <!-- Card: Input IP Perangkat -->
    <div class="w-full bg-gray-800/70 backdrop-blur-md rounded-2xl p-5 shadow-md border border-gray-700">
      <h2 class="text-lg font-semibold text-indigo-400 mb-4 text-center">IP Perangkat</h2>
      <div class="flex gap-3 justify-center">
        <input v-model="deviceIp" placeholder="Masukkan IP perangkat (misal 192.168.1.123)"
          class="flex-1 px-4 py-2 rounded-lg bg-gray-900 border border-gray-700 text-white focus:outline-none focus:ring-2 focus:ring-indigo-500" />
        <button @click="checkDevice"
          class="px-5 py-2 rounded-lg bg-indigo-600 hover:bg-indigo-700 font-semibold shadow hover:shadow-indigo-500/40 transition">
          Check
        </button>
      </div>
    </div>

    <!-- Card: Tombol Utama -->
    <div class="w-full bg-gray-800/70 backdrop-blur-md rounded-2xl p-5 shadow-md border border-gray-700 text-center">
      <div class="flex justify-center gap-4">
        <button @click="showTimeOnDevice"
          class="px-6 py-3 bg-indigo-500 hover:bg-indigo-600 rounded-lg font-semibold shadow hover:shadow-indigo-500/40 transition">
          Tampilkan Jam
        </button>

        <button @click="restoreNormalDisplay"
          class="px-6 py-3 bg-yellow-500 hover:bg-yellow-600 rounded-lg font-semibold shadow hover:shadow-yellow-500/40 transition">
          Cocokkan Otomatis
        </button>
      </div>
    </div>

    <!-- Card: Atur Jam Manual (hanya jam saja) -->
    <div class="w-full bg-gray-800/70 backdrop-blur-md rounded-2xl p-5 shadow-md border border-gray-700">
      <h3 class="text-md font-semibold text-indigo-300 mb-4 text-center">Atur Jam Manual</h3>
      <div class="flex flex-col gap-3">
        <input v-model="manualTime" type="time"
          class="px-4 py-2 rounded-lg bg-gray-900 border border-gray-700 text-white focus:outline-none focus:ring-2 focus:ring-indigo-500" />
      </div>
      <div class="flex justify-center gap-4 mt-4">
        <button @click="setManual"
          class="px-5 py-2 bg-blue-500 hover:bg-blue-600 rounded-lg font-semibold shadow hover:shadow-blue-500/40 transition">
          Set ke Alat
        </button>
      </div>
    </div>

    <!-- Modal & Toast tetap sama -->
    <transition name="fade">
      <teleport to="body">
        <div v-if="showConfirmModal"
          class="fixed inset-0 flex items-center justify-center bg-black/70 backdrop-blur-sm z-[9999]">
          <div
            class="relative bg-gray-900/95 rounded-2xl p-8 w-[400px] text-center shadow-2xl border border-indigo-500/40 animate-zoomIn">
            <button @click="closeModal"
              class="absolute top-2 right-3 text-gray-400 hover:text-white text-2xl font-bold transition">×</button>
            <h3 class="text-xl font-semibold text-indigo-400 mb-3">Konfirmasi Tindakan</h3>
            <p class="text-gray-300 mb-6 text-sm leading-relaxed">
              Apakah kamu ingin menampilkan teks di website saja atau mengirimkannya ke alat?
            </p>
            <div class="flex justify-center gap-3">
              <button @click="previewWeb"
                class="px-4 py-2 bg-blue-500 hover:bg-blue-600 rounded-lg font-semibold shadow hover:shadow-blue-500/40 transition">Preview
                di Web</button>
              <button @click="sendToDevice"
                class="px-4 py-2 bg-green-500 hover:bg-green-600 rounded-lg font-semibold shadow hover:shadow-green-500/40 transition">Kirim
                ke Alat</button>
            </div>
            <button @click="closeModal"
              class="mt-6 text-sm text-gray-400 hover:text-gray-200 underline transition">Batal</button>
          </div>
        </div>
      </teleport>
    </transition>

    <teleport to="body">
      <transition name="fade">
        <div v-if="toastMessage"
          :class="['fixed bottom-6 right-6 flex items-center gap-3 px-4 py-3 rounded-lg shadow-lg border backdrop-blur-md min-w-[250px] z-[99999]',
            toastType === 'success' ? 'bg-green-600/80 border-green-400 text-white' : 'bg-red-600/80 border-red-400 text-white']">
          <span class="font-medium">{{ toastMessage }}</span>
        </div>
      </transition>
    </teleport>

  </div>
</template>

<script setup>
import { ref, watch } from 'vue'

const deviceIp = ref(localStorage.getItem('deviceIp') || '')
const showConfirmModal = ref(false)
const toastMessage = ref(null)
const toastType = ref('success')

//  Properti untuk Atur Jam Manual (hanya jam)
const manualTime = ref('')

const emit = defineEmits([
  'showTime',
  'setManualTime',
  'syncAutoTime',
  'deviceConnected',
])


watch(deviceIp, (val) => localStorage.setItem('deviceIp', val))

const checkDevice = async () => {
  if (!deviceIp.value) return showToast('Isi IP perangkat dulu', 'error')
  try {
    const controller = new AbortController()
    const timeout = setTimeout(() => controller.abort(), 3000)
    const res = await fetch(`http://${deviceIp.value}/status`, { signal: controller.signal })
    clearTimeout(timeout)
    if (!res.ok) throw new Error('Tidak bisa diakses')
    const data = await res.json()
    showToast('Perangkat online: ' + data.time, 'success')
    emit('deviceConnected', deviceIp.value, true)
  } catch {
    emit('deviceConnected', deviceIp.value, false)
  }
}

const showTimeOnDevice = async () => {
  if (!deviceIp.value) return showToast('Isi IP perangkat dulu', 'error')
  try {
    await fetch(`http://${deviceIp.value}/display-time-only?mode=1`)
    showToast('Mode Jam Aktif', 'success')
    emit('showTime')
  } catch {
    showToast('Gagal mengubah mode', 'error')
  }
}

const setManual = async () => {
  if (!deviceIp.value) return showToast('Isi IP perangkat dulu', 'error')
  if (!manualTime.value) return showToast('Isi jam manual dulu', 'error')

  try {
    await fetch(`http://${deviceIp.value}/set-time?time=${manualTime.value}`)
    showToast(`Jam diatur ke ${manualTime.value}`, 'success')
    emit('setManualTime', { time: manualTime.value })
  } catch {
    showToast('Gagal mengatur jam manual', 'error')
  }
}


const restoreNormalDisplay = async () => {
  if (!deviceIp.value) return showToast('Isi IP perangkat dulu', 'error')
  try {
    await fetch(`http://${deviceIp.value}/sync-ntp`) // ✅ sinkron NTP
    await fetch(`http://${deviceIp.value}/display-time-only?mode=0`)
    showToast('Waktu disinkron otomatis', 'success')
    emit('syncAutoTime')
  } catch {
    showToast('Gagal sinkron otomatis', 'error')
  }
}

const showToast = (msg, type = 'success') => {
  toastMessage.value = msg
  toastType.value = type
  setTimeout(() => (toastMessage.value = null), 3000)
}
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
