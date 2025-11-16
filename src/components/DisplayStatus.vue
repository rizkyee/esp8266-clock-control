<template>
  <div
    class="bg-gray-900/80 backdrop-blur-sm p-6 rounded-2xl shadow-inner w-full max-w-4xl mb-8 border border-gray-700 text-center mx-auto">
    <h2 class="text-lg font-semibold text-indigo-400 mb-3 uppercase tracking-wider">
      Status Tampilan
    </h2>

    <div class="relative overflow-hidden rounded-xl border border-gray-700 bg-black py-6 px-8"
      style="width: 100%; min-height: 120px;">
      <!-- Ukuran font dikurangi agar tidak melebihi batas -->
      <div class="whitespace-nowrap text-red-500 text-2xl md:text-3xl font-mono tracking-widest leading-tight"
        :style="{ textShadow: '0 0 8px #ff0000, 0 0 20px #ff5555' }">
        {{ displayText }}
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, watch, onMounted } from 'vue'

const props = defineProps({
  deviceIp: {
    type: String,
    default: () => localStorage.getItem('deviceIp') || ''
  },
  displayText: String
})

const displayText = ref('Menunggu koneksi ke perangkat...')
const lastData = ref(null)
const showOnlyTime = ref(false)

async function ambilDataDariAlat() {
  if (!props.deviceIp) {
    displayText.value = 'IP perangkat belum diisi'
    return
  }

  try {
    const controller = new AbortController()
    const timeout = setTimeout(() => controller.abort(), 3000)
    const res = await fetch(`http://${props.deviceIp}/status`, { signal: controller.signal })
    clearTimeout(timeout)

    if (!res.ok) throw new Error('Respon tidak valid')
    const data = await res.json()

    lastData.value = data

    if (data.temperature === undefined || data.humidity === undefined) {
      showOnlyTime.value = true
      displayText.value = data.time.length > 5 ? data.time.slice(0, 5) : data.time
    } else {
      showOnlyTime.value = false
      displayText.value = `${data.time} | T:${data.temperature}°C | H:${data.humidity}%`
    }
  } catch {
    displayText.value = 'Tidak bisa terhubung ke perangkat'
    lastData.value = null
  }
}

function updateLocalTime() {
  if (!lastData.value || !lastData.value.time) return
  const now = new Date()
  const [hour, minute] = lastData.value.time.split(':')

  if (showOnlyTime.value) {
    displayText.value = `${hour}:${minute}`
  } else {
    const second = now.getSeconds().toString().padStart(2, '0')
    displayText.value = `${hour}:${minute}:${second} | T:${lastData.value.temperature ?? '-'}°C | H:${lastData.value.humidity ?? '-'}%`
  }
}

watch(
  () => props.deviceIp,
  async (newIp) => {
    if (!newIp) {
      displayText.value = 'IP perangkat belum diisi'
      lastData.value = null
      return
    }

    displayText.value = 'Menyambungkan ke perangkat...'
    lastData.value = null
    await ambilDataDariAlat()
  }
)

onMounted(() => {
  ambilDataDariAlat()
  setInterval(ambilDataDariAlat, 5000)
  setInterval(updateLocalTime, 1000)
})
</script>
