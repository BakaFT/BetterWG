# 总览

这是一个通过 DLL Hijacking 对 CEF 项目进行 Hook 的项目

> 本文假设读者已经了解 WeGame 的接口中返回了一个未被渲染的`teamElo`字段，并且了解 DevTools的用法

## 通过DLL Hijacking的方式注入进程

**这是自定义代码得以注入运行的根本**

由于 Windows 系统中存在 [动态链接库搜索顺序](https://learn.microsoft.com/zh-cn/windows/win32/dlls/dynamic-link-library-search-order) 的设计，根目录的`dwrite.dll`会优先于系统目录下真正的`dwrite.dll`，从而使得我们自己编写的代码可以趁机注入到目标进程中

而又因为程序确实要用到`dwrite.dll`导出的函数，所以需要在`dllproxy.cc`中实现对其导出函数的转发，避免程序报错

> 如果你有玩老单机装 MOD 的经历，很多 MOD 加载器名字都叫`d3d9.dll`，正是运用了这个技巧

## 对请求进行拦截修改

通过继承`cef_response_filter_t` 类**对 WeGame 的前端请求的响应体进行修改**

这里选择了**拦截请求后修改Response Body**，而非在运行时（确切地说，在Renderer进程中）注入新的JS代码，主要是因为：

1. WeGame 的前端部分比较微妙，每个游戏的业务页面都是一个`iframe` ，并列排布在一个父页面里，跨`iframe`操作容易受限，不太想尝试
2. WeGame 的前端业务在 WebPack 打包之后删掉了 SourceMap, 不好做 JS 层的 Hook

### 说一下怎么显示TeamELO的

首先要考虑在哪里展示数据，盯着 WeGame 的界面寻思了一下，把这个数据放在表头应该不错

通过开发者工具进行摸索（什么，你没有？看下面开启远程调试的部分），找到战绩页面表头的渲染逻辑存在于这里 ->`https://wegame.gtimg.com/g.26-r.c2d3c/helper/lol/v2/assets/battle-detail-base.4c9a3294.js` （你可以通过搜索表头里面汉字对应的Unicode字节序列找到）

观察下面的节选代码（已经适当格式化）：

```javascript
 Y(P(et), {
                text: "\u6467\u6BC1\u53EC\u5524\u6C34\u6676"
            }, {
                default: Q(()=>[h("div", An, [$n, h("span", null, w(o.battleDetailTeamData.totalDampenKilled), 1)])]),
                _: 1
            }), Y(P(et), {
                text: "\u51FB\u6740\u7537\u7235"
            }, {
                default: Q(()=>[h("div", zn, [kn, h("span", null, w(o.battleDetailTeamData.totalBaronKills), 1)])]),
                _: 1
            }), Y(P(et), {
                text: "\u51FB\u6740\u5DE8\u9F99"
            }, {
                default: Q(()=>[h("div", Bn, [Pn, h("span", null, w(o.battleDetailTeamData.totalDragonKills), 1)])]),
                _: 1
            }), !((l = t.battleDetailTeamData) == null ? void 0 : l.placement) && o.battleDetailTeamData.banChampionList.length ? (b(),
```

如果你写过 Vue.js，你会发现，这里用到了`h` 函数，并且可以直接访问到 `teamElo`所在的对象`battleDetailTeamData`

照葫芦画瓢，写出如下代码

`"Y(P(et),{text:'TeamELO'},{default:Q(()=>[h('div',Bn,['TeamELO:',h('span',null,w(o.battleDetailTeamData.teamElo),1)])]),_:1}),`

插入到`!((l = t.battleDetailTeamData)`之前即可

> 如果是在浏览器里的话写个油猴脚本就秒了，但是浏览器里看的话不能换区只能搜ID，用起来还是比较麻烦

> 费这么大劲只是为了插这么一小段代码，闹麻了

## 开启远程调试

DevTools是C/S架构，核心协议是 [Chrome DevTools Protocol](https://chromedevtools.github.io/devtools-protocol/)，我们平时F12看到的是 DevTools 的前端，浏览器中运行着真正的服务端

由于 WeGame 把 `http://localhost:DEBUGGING_PORT/devtools/inspector.html` 直接给 trim 掉了，我猜内置的 DevTools 可能已经打不开了（因为实质上的**客户端**被砍了。猜测，并未尝试，如有大佬请务必在 Issues 不吝赐教）

所以换个思路，开启 Remote Debugging Port（本质上是给进程加参数`--remote-debugging-port=PORT`)，先把服务端开了再说

操作的话就是对`cef_initialize`进行 Hook，拿到`cef_app_t`对象之后，再对其`on_before_command_line_processing`进行 Hook，这个函数的第三个参数就是程序的命令行 `cef_command_line_t`，加一个`--remote-debugging-port=PORT` 即可开启 DevTools 的服务端

这时候打开`http://localhost:REMOTE_DEBUGGING_PORT/json`，你会看到一个类似这样的响应

```json
[
    {
        "description": "",
        "devtoolsFrontendUrl": "/devtools/inspector.html?ws=localhost:29929/devtools/page/A70111D5A451CC88164DDDBCE65EB546",
        "id": "A70111D5A451CC88164DDDBCE65EB546",
        "title": "WeGame",
        "type": "page",
        "url": "https://www.wegame.com.cn/frames/index.html",
        "webSocketDebuggerUrl": "ws://localhost:29929/devtools/page/A70111D5A451CC88164DDDBCE65EB546"
    }
]
```

按照这个响应，你打开 `http://localhost:REMOTE_DEBUGGING_PORT/devtools/inspector.html?ws=localhost:29929/devtools/page/A70111D5A451CC88164DDDBCE65EB546`就可以看到熟悉的调试界面了，但是问题是，这个内置的前端页面被删掉了

那就**借用**一下 Chrome/MS Edge 的就好，在这里 -> `devtools://devtools/bundled/inspector.html`，浏览器里打开 `devtools://devtools/bundled/inspector.html?ws://localhost:29929/devtools/page/A70111D5A451CC88164DDDBCE65EB546` ，效果是一样的







