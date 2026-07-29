---
name: cubeMX-vs-eide-source-files
description: CubeMX 配新外设后，EIDE 不会自动同步源文件和 include path，每次都要手动补
metadata:
  type: project
---

项目使用 CubeMX + EIDE (VS Code 插件) 开发 STM32G474。CubeMX 生成代码时只更新 `.uvprojx` (Keil 工程)，但 EIDE 实际的编译文件列表在 `.eide/eide.yml` 中。

## 典型症状

CubeMX 配置新外设（如 TIM、UART、SPI）后编译报错：
```
Error: L6218E: Undefined symbol HAL_xxx_Init (referred from main.o)
Error: L6218E: Undefined symbol HAL_xxx_xxx (referred from ...)
```

## 原因

CubeMX 更新了 MDK-ARM/project.uvprojx 里的文件列表，但 EIDE 只读 `.eide/eide.yml`，两个文件完全不同步。

## 修复步骤

每次 CubeMX 配置新外设模块后，检查并手动编辑 `.eide/eide.yml`：

1. **添加 HAL 驱动源文件** — 在 `Drivers/STM32G4xx_HAL_Driver` 节点下，加上对应的 `.c` 文件：
   ```yaml
   - path: Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_<module>.c
   - path: Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_<module>_ex.c
   ```

2. **添加用户源文件** — Lib/ 或自定义目录下的 `.c` 文件也要加。

3. **添加 Include Path** — 如果新增了头文件目录，在 `incList` 下加路径。

4. **Rebuild**，不是 Build。Build 是增量编译，没有旧的 `.o` 文件会跳过。

**How to apply:** 每次 CubeMX 生成代码后，用 `git diff MDK-ARM/project.uvprojx` 看 CubeMX 加了哪些新文件，然后同步补到 `.eide/eide.yml` 里。
