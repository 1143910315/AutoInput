# 项目说明

这个项目是基于 Xmake 构建系统，前端采用 pnpm，前端框架选用 Vue，UI 框架则是 Element，开发语言为 C++。

## 推荐的开发环境

建议使用 [VSCode](https://code.visualstudio.com/) 作为开发环境，结合合适的插件来提升开发体验。

## 项目设置

安装xmake和pnpm

接下来，进入项目根目录，通过 Xmake 进行构建：

```sh
xmake
```

## 开发

### 开发环境启动

```sh
pnpm dev
```

### 生产环境构建

```sh
pnpm build
```

### 单元测试

使用 [Vitest](https://vitest.dev/) 进行单元测试：

```sh
pnpm test:unit
```

### 代码检查

使用 [ESLint](https://eslint.org/) 进行代码检查：

```sh
pnpm lint
```

## 自定义配置

有关详细的配置选项，请参考 [Xmake 文档](https://xmake.io/#/)。