# MultiVersionPlugin

LeviLamina плагин — мультиверсийная поддержка клиентов **1.21.5x – 1.26.1x** поверх библиотеки [SculkCatalystMC/Protocol](https://github.com/SculkCatalystMC/Protocol).

> **Protocol уже установлен на сервере** в папке `libraries/` — плагин использует только его заголовки при компиляции, `.dll`/`.so` уже там.

## Поддерживаемые протоколы

| Версия Minecraft | Protocol ID | Статус            |
|-----------------|-------------|-------------------|
| 1.21.5x         | 766         | ✅ Поддерживается |
| 1.21.6x         | 776         | ✅ Поддерживается |
| 1.21.7x         | 786         | ✅ Поддерживается |
| 1.21.8x         | 800         | ✅ Поддерживается |
| 1.21.9x         | 818, 819    | ✅ Поддерживается |
| 1.21.10x        | 827         | ✅ Поддерживается |
| 1.21.11x        | 844         | ✅ Поддерживается |
| 1.21.12x        | 859, 860    | ✅ Поддерживается |
| 1.21.13x        | 898         | ✅ Поддерживается |
| 1.26.x          | 924         | ✅ Поддерживается |
| 1.26.1x         | 944         | ⏳ Скоро (ProtocolLib) |

## Архитектура

```
BDS (1.26.10)
  └── LeviLamina (mod loader)
        └── MultiVersionPlugin.dll   ← этот плагин
              ├── PacketHook         — хуки Login / Disconnect / sendToClient
              ├── VersionRegistry    — реестр protocol ID
              ├── ClientVersionMap   — per-client протокол (потокобезопасно)
              └── PacketTranslator   — трансляция через sculk::protocol

libraries/                           ← на сервере, плагин сюда не кладёт
  └── Protocol.dll / Protocol.so
```

## Сборка через GitHub Actions (рекомендуется)

1. Создайте репозиторий, закиньте содержимое этой папки
2. Запушьте в `main` — workflow запустится автоматически
3. Actions сам скачает только `include/` из Protocol для компиляции
4. Готовый `.dll` будет в артефактах билда

Для релиза:
```bash
git tag v0.1.0 && git push --tags
```

## Локальная сборка

Требования: Visual Studio 2022 (MSVC v143), xmake.

```cmd
# Скачиваем только заголовки Protocol
git clone --depth 1 --filter=blob:none --sparse https://github.com/SculkCatalystMC/Protocol.git vendor/Protocol
cd vendor/Protocol && git sparse-checkout set include && cd ../..

# Собираем
set PROTOCOL_INCLUDE_DIR=vendor/Protocol/include
xmake f -y -p windows -a x64 -m release
xmake
```

DLL будет в `build/windows/x64/release/MultiVersionPlugin.dll`.

## Установка на сервер

```
plugins/
  MultiVersionPlugin/
    MultiVersionPlugin.dll    ← из артефакта GitHub Actions
    manifest.json             ← из этого репо
```

Protocol `.dll`/`.so` уже лежит в `libraries/` — ничего дополнительно устанавливать не нужно.

## Требования

- LeviLamina ≥ 1.0.0
- BDS 1.26.10
- [GlacieTeam/ProtocolLib](https://github.com/GlacieTeam/ProtocolLib) в `libraries/`
- [SculkCatalystMC/Protocol](https://github.com/SculkCatalystMC/Protocol) в `libraries/`

## Лицензия

MIT
