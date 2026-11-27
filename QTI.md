# 🎧 ResonanceFX QTI — OnePlus 15 / crDroid A16

![Versión](https://img.shields.io/badge/ResonanceFX-v2.9.0--qti-7C4DFF?style=for-the-badge)
![Android](https://img.shields.io/badge/Android-16%20(API%2036)-3DDC84?style=for-the-badge&logo=android&logoColor=white)
![ROM](https://img.shields.io/badge/ROM-crDroid%2012.11-orange?style=for-the-badge)
![Root](https://img.shields.io/badge/Root-KernelSU%20Next%20(Wild)-blue?style=for-the-badge)
![HAL](https://img.shields.io/badge/HAL-AIDL%20QTI%20(vendor)-red?style=for-the-badge)
![Estado](https://img.shields.io/badge/Estado-Funcionando%20✅-success?style=for-the-badge)
![Shell](https://img.shields.io/badge/Termux-fish%20shell-4AAE47?style=for-the-badge)

Fork de [ResonanceFX](https://github.com/xeloria/ResonanceFX) (Xeloria) con dos parches para que funcione en el HAL de audio AIDL de Qualcomm (`audiohalservice.qti`) del OnePlus 15 con crDroid Android 16.

---

## 📦 Dónde está todo

| Qué | Dónde |
|---|---|
| 🍴 Fork | `github.com/BlackArchmm/ResonanceFX` (rama `main`) |
| 🧬 Original del autor | `github.com/xeloria/ResonanceFX` (remoto `upstream`) |
| 🏷️ Releases permanentes | `qti-2.9.0` (actual) · `qti-2.6.0` (anterior) |
| 🛟 Rama de respaldo | `backup-2.6.0-qti` (la 2.6.0 con los 6 parches viejos) |
| ⚙️ Workflow | `.github/workflows/build-qti.yml` (se lanza a mano) |
| 💻 Clon local | `~/resonance-build/rfx-fork` (Termux) |
| 📱 Copia local | `/sdcard/DEVELOPER/PROYECT-S/ResonanceFX-QTI/` |
| 🧩 V3 del vendor | `vendor-V3-ndk.so`, hash `adb55a59…d56ecde3` |

> [!WARNING]
> Los artifacts de GitHub Actions **caducan a los 90 días**. Las copias buenas son las **releases** y la carpeta local.

---

## 🛠️ Qué lleva este fork sobre la 2.9.0 del autor

| | Cambio | Archivo | Script |
|---|---|---|---|
| 🧩 | **V3 del vendor** del OnePlus (en vez de la de Samsung) | `engine/src/android/aidl/prebuilt/arm64-v8a/` | — |
| ⚙️ | **Workflow** `build-qti.yml` (módulo + APK) | `.github/workflows/` | — |
| 🏷️ | Sufijo **`-qti`** en la versión | `module/module.prop` | — |
| 🛡️ | **Parche 4**: memoria FMQ con la `libcutils` del sistema y **sin `abort`** si falla (bypass en vez de quedarse sin sonido) | `NativeHandle.cpp`, `EffectContext.cpp`, `EffectImpl.cpp` | `rfx_qti_fix4.py` |
| 📂 | **Parche 6**: el montaje de respaldo del motor AIDL va a la partición detectada (`/vendor`), no a `/system` fijo | `module/common.sh` | `rfx_qti_fix6.py` |

### ✅ Lo que ya trae el autor (no hace falta parchear)

| Arreglo | Desde |
|---|---|
| 🔥 Vtable alineada (`binder_shell.h`) | 2.9.0 |
| Motor AIDL en `/vendor` en HAL QTI (detección) | 2.9.0 |
| Reglas SELinux para `hal_audio_default` | 2.9.0 |
| Veredicto ✅ en KernelSU con HAL AIDL | 2.9.0 |
| Compilación con `c++_static` | siempre |

> [!NOTE]
> El parche 6 corrige un **fallo de la 2.9.0**: detecta `/vendor` pero la comprobación y el montaje siguen usando `/system/lib64/soundfx`, que en esta ROM no existe. Sin él, el HAL dice `library resonancefx_aidl does not exist!` (código **E03**/**E06**).

---

## 🔄 Cuando el autor saque una versión nueva

> [!CAUTION]
> **No instales el zip oficial.** Sustituye al tuyo (mismo id `resonancefx`) y sin los parches no suena. Actualiza siempre a través del fork.

### 0️⃣ Antes de nada: copia de la configuración de la app

```fish
su -c 'tar -czf /sdcard/DEVELOPER/PROYECT-S/ResonanceFX-QTI/rfx-appdata.tgz -C /data/data io.github.resonancefx'
```

> [!TIP]
> Además, en la app: **Preajustes → guarda** tu configuración → **icono de exportar** (flecha ↑) al lado del preset → JSON.

### 1️⃣ Pasar el fork a la versión nueva

Cambia `X.Y.Z` por la versión que tienes ahora (para nombrar el respaldo):

```fish
cd ~/resonance-build/rfx-fork; and git branch -f backup-X.Y.Z-qti; and git push origin backup-X.Y.Z-qti; and git fetch upstream --tags; and git reset --hard upstream/main; and git checkout backup-X.Y.Z-qti -- .github/workflows/build-qti.yml engine/src/android/aidl/prebuilt/arm64-v8a/android.hardware.audio.effect-V3-ndk.so QTI.md
```

### 2️⃣ Aplicar los parches

```fish
python3 ~/resonance-build/rfx_qti_fix4.py; and python3 ~/resonance-build/rfx_qti_fix6.py; and sed -i 's/^version=\(.*\)$/version=\1-qti/' module/module.prop; and grep version= module/module.prop; and git status --short
```

| Si un script dice… | Significa |
|---|---|
| `OK` | Aplicado ✅ |
| `ya estaba parcheado` | El autor lo arregló 🎉: ya no hace falta |
| `no encuentro … como esperaba` | El autor cambió ese código: hay que adaptar el parche (pásale el error a Claude) |

> [!IMPORTANT]
> Los scripts viven en `~/resonance-build/` y en la carpeta del proyecto. Si no están en `~/resonance-build/`, cópialos antes.

### 3️⃣ Subir y compilar

```fish
git add -A; and git commit -m "QTI on vX.Y.Z"; and git push --force origin main; and gh workflow run build-qti.yml -R BlackArchmm/ResonanceFX; and sleep 5; and gh run list -R BlackArchmm/ResonanceFX -w build-qti.yml -L 1
```

Apunta el **ID completo** del run (todo el número, no solo el principio).

### 4️⃣ Descargar e instalar el módulo encima

```fish
gh run watch ID -R BlackArchmm/ResonanceFX --exit-status; and rm -rf /sdcard/rfx/nuevo; and gh run download ID -R BlackArchmm/ResonanceFX -n ResonanceFX-qti -D /sdcard/rfx/nuevo; and su -c 'ksud module install /sdcard/rfx/nuevo/ResonanceFX-*-qti.zip 2>&1 | tail -n 1'
```

Debe salir `Module installed successfully!`.

### 5️⃣ La APK

| Caso | 📲 Qué hacer |
|---|---|
| El changelog no toca la app | No la cambies |
| La app trae novedades | Copia de datos (paso 0) → desinstalar → instalar la nueva → restaurar datos |

```fish
su -c 'pm uninstall io.github.resonancefx; cp /sdcard/rfx/nuevo/apk/*.apk /data/local/tmp/rfx.apk; pm install /data/local/tmp/rfx.apk; rm -f /data/local/tmp/rfx.apk'
```

**Restaurar la configuración** (con la app cerrada):

```fish
su -c 'U=$(stat -c %u /data/data/io.github.resonancefx); am force-stop io.github.resonancefx; tar -xzf /sdcard/DEVELOPER/PROYECT-S/ResonanceFX-QTI/rfx-appdata.tgz -C /data/data; chown -R $U:$U /data/data/io.github.resonancefx; restorecon -R /data/data/io.github.resonancefx'
```

> [!WARNING]
> Cada build firma la APK con una clave distinta: **no se actualiza encima**, hay que desinstalar. Sin la copia del paso 0, **se pierde la configuración**.

### 6️⃣ Reiniciar y comprobar

```fish
su -c 'grep -c resonancefx /proc/$(pidof audiohalservice.qti)/maps; grep -E "library dir|HAL sees" /data/adb/resonancefx/logs/boot.log | tail -n 3'
```

| ✅ Bien | ❌ Mal |
|---|---|
| Número **mayor que 0** | `0` |
| `bind-mounted library dir /vendor/lib64/soundfx` | Sin línea `library dir` |
| `audio HAL sees /vendor/lib64/soundfx/…` | `E03` / `E06` en la app |

> [!NOTE]
> En una **instalación desde cero** puede hacer falta **reiniciar dos veces**: la primera vez el módulo descubre que el HAL es de vendor y lo guarda para el siguiente arranque.

### 7️⃣ Guardar copia

```fish
gh release create qti-X.Y.Z /sdcard/rfx/nuevo/ResonanceFX-*-qti.zip /sdcard/rfx/nuevo/apk/*.apk -R BlackArchmm/ResonanceFX --target main --title "ResonanceFX X.Y.Z QTI (OnePlus 15 / crDroid A16)" --notes "X.Y.Z del autor + parches QTI 4 y 6."; and cp /sdcard/rfx/nuevo/ResonanceFX-*-qti.zip /sdcard/DEVELOPER/PROYECT-S/ResonanceFX-QTI/
```

> [!NOTE]
> El tag empieza por `qti-` a propósito: así no dispara el `release.yml` del autor.

---

## 🚑 Si algo va mal

| Situación | Qué hacer |
|---|---|
| 🔇 Sin sonido | `su -c 'ksud module disable resonancefx'` y reiniciar |
| 🧱 No arranca | Desde recovery, borrar `/data/adb/modules/resonancefx` |
| ⏪ Volver a la versión anterior | Instalar el zip de la release anterior (`qti-2.9.0`, `qti-2.6.0`) |
| 🔄 OTA de crDroid y deja de ir | Comparar el hash de `/vendor/lib64/android.hardware.audio.effect-V3-ndk.so`. Si cambió, copiarla a `engine/src/android/aidl/prebuilt/arm64-v8a/` y recompilar (pasos 3, 4, 6) |
| 🎵 Una app no aplica el efecto | Apps de DJ/producción (Remixlive) usan audio **RAW/MMAP** y se saltan los efectos por diseño. Reproductores con EQ propio: desactivarlo |
| 🎧 Bluetooth | Funciona. Cada dispositivo tiene **su propio perfil**: hay que configurarlo la primera vez |

> [!WARNING]
> No pulses **Acción** del módulo si todo funciona: reinicia el audio y no aporta nada.

---

## 🔍 Diagnóstico

```fish
# Motor cargado en el HAL (> 0 = sí)
su -c 'grep -c resonancefx /proc/$(pidof audiohalservice.qti)/maps'

# Log de arranque del módulo
su -c 'tail -n 20 /data/adb/resonancefx/logs/boot.log'

# Log del motor
su -c 'logcat -d | grep -E "ResonanceFX|EffectFactoryQti" | tail -n 40'

# Partición elegida para el AIDL (debe ser /vendor)
su -c 'cat /data/adb/resonancefx/state/aidl_partition'

# Por qué hilo sale el audio (MIXER = bien; RAW/MMAP/OFFLOAD = sin efecto)
su -c 'dumpsys media.audio_flinger | grep -E "^Output thread|AudioStreamOut:|Standby:|name: ResonanceFX"'

# Hash de la V3 del vendor (adb55a59…)
su -c 'sha256sum /vendor/lib64/android.hardware.audio.effect-V3-ndk.so'
```

---

## ✅ Checklist

**📥 Instalar desde cero**
- [ ] Flashear `ResonanceFX-vX.Y.Z-qti.zip` en KernelSU
- [ ] Reiniciar (y **otra vez** si sale E03/E06)
- [ ] Instalar la APK y restaurar `rfx-appdata.tgz` si hay copia
- [ ] App → Ajustes → **Todo el audio** → interruptor activado
- [ ] Estado: ✓ y `48000 Hz · 2 canales`
- [ ] KernelSU: ResonanceFX en ✅

**🔄 Actualizar a una versión nueva del autor**
- [ ] Copia de la configuración de la app (paso 0)
- [ ] Rama de respaldo + `reset` a `upstream/main` + recuperar workflow, V3 y `QTI.md`
- [ ] `rfx_qti_fix4.py` y `rfx_qti_fix6.py` → `OK` (o "ya estaba parcheado")
- [ ] Versión con `-qti`
- [ ] `push --force` + `gh workflow run build-qti.yml` → ✅
- [ ] Instalar el zip encima (nunca el oficial)
- [ ] APK solo si hace falta, con copia y restauración de datos
- [ ] Reiniciar y comprobar el paso 6
- [ ] Release `qti-X.Y.Z` y copia en la carpeta del proyecto

**🚑 Rescate**
- [ ] Sin sonido → `ksud module disable resonancefx` y reiniciar
- [ ] Sin arranque → borrar `/data/adb/modules/resonancefx` desde recovery
