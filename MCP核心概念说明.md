# Model Context Protocol (MCP) Python SDK 核心概念详解

## 目录

- [概述](#概述)
- [什么是 MCP？](#什么是-mcp)
- [核心概念](#核心概念)
  - [1. Server (服务器)](#1-server-服务器)
  - [2. Resources (资源)](#2-resources-资源)
  - [3. Tools (工具)](#3-tools-工具)
  - [4. Prompts (提示词模板)](#4-prompts-提示词模板)
  - [5. Context (上下文)](#5-context-上下文)
  - [6. Images (图像)](#6-images-图像)
  - [7. Completions (自动完成)](#7-completions-自动完成)
  - [8. Elicitation (用户交互请求)](#8-elicitation-用户交互请求)
  - [9. Sampling (采样)](#9-sampling-采样)
  - [10. Logging and Notifications (日志和通知)](#10-logging-and-notifications-日志和通知)
  - [11. Authentication (认证)](#11-authentication-认证)
- [MCP 三大基础原语](#mcp-三大基础原语)
- [服务器能力](#服务器能力)
- [运行服务器](#运行服务器)
- [总结](#总结)

## 概述

Model Context Protocol (MCP) Python SDK 是一个用于实现模型上下文协议的 Python 库。它允许应用程序以标准化的方式为大型语言模型（LLM）提供上下文信息，将提供上下文的职责与实际的 LLM 交互分离开来。

**源地址**: https://github.com/modelcontextprotocol/python-sdk

## 什么是 MCP？

Model Context Protocol (MCP) 是一个开放协议，允许你构建能够以安全、标准化的方式向 LLM 应用程序公开数据和功能的服务器。可以把它想象成专门为 LLM 交互设计的 Web API。

### MCP 的核心功能

1. **暴露数据** - 通过 **Resources（资源）** 暴露数据（类似 GET 端点；用于将信息加载到 LLM 的上下文中）
2. **提供功能** - 通过 **Tools（工具）** 提供功能（类似 POST 端点；用于执行代码或产生副作用）
3. **定义交互模式** - 通过 **Prompts（提示词模板）** 定义 LLM 交互的可重用模板
4. **更多功能** - 还包括日志记录、采样、认证等高级功能

## 核心概念

### 1. Server (服务器)

**Server** 是 MCP 协议的核心接口。它处理连接管理、协议合规性和消息路由。

#### FastMCP 服务器

FastMCP 是高级服务器实现，提供了简单易用的 API：

```python
from mcp.server.fastmcp import FastMCP

# 创建一个 MCP 服务器
mcp = FastMCP("Demo", json_response=True)

# 添加工具
@mcp.tool()
def add(a: int, b: int) -> int:
    """将两个数字相加"""
    return a + b

# 运行服务器
if __name__ == "__main__":
    mcp.run(transport="streamable-http")
```

**关键特性：**
- 自动处理协议细节
- 支持多种传输方式（stdio、SSE、HTTP）
- 内置生命周期管理
- 简单的装饰器 API

#### 生命周期管理

服务器支持生命周期管理，用于初始化和清理资源：

```python
from contextlib import asynccontextmanager
from collections.abc import AsyncIterator
from dataclasses import dataclass

class Database:
    @classmethod
    async def connect(cls) -> "Database":
        return cls()
    
    async def disconnect(self) -> None:
        pass

@dataclass
class AppContext:
    """应用上下文，包含类型化的依赖项"""
    db: Database

@asynccontextmanager
async def app_lifespan(server: FastMCP) -> AsyncIterator[AppContext]:
    """管理应用生命周期"""
    # 启动时初始化
    db = await Database.connect()
    try:
        yield AppContext(db=db)
    finally:
        # 关闭时清理
        await db.disconnect()

# 将生命周期传递给服务器
mcp = FastMCP("My App", lifespan=app_lifespan)
```

### 2. Resources (资源)

**Resources** 是向 LLM 暴露数据的方式。它们类似于 REST API 中的 GET 端点 - 提供数据但不应该执行大量计算或产生副作用。

#### 基本用法

```python
from mcp.server.fastmcp import FastMCP

mcp = FastMCP(name="Resource Example")

@mcp.resource("file://documents/{name}")
def read_document(name: str) -> str:
    """根据名称读取文档"""
    return f"Content of {name}"

@mcp.resource("config://settings")
def get_settings() -> str:
    """获取应用设置"""
    return """{
  "theme": "dark",
  "language": "en",
  "debug": false
}"""
```

**关键特性：**
- URI 模板支持动态参数（如 `{name}`）
- 用于读取文件、配置、数据库记录等
- 应该是幂等的（多次调用返回相同结果）
- 不应该产生副作用

### 3. Tools (工具)

**Tools** 让 LLM 通过服务器执行操作。与 Resources 不同，Tools 预期会执行计算并产生副作用。

#### 基本工具示例

```python
from mcp.server.fastmcp import FastMCP

mcp = FastMCP(name="Tool Example")

@mcp.tool()
def sum(a: int, b: int) -> int:
    """将两个数字相加"""
    return a + b

@mcp.tool()
def get_weather(city: str, unit: str = "celsius") -> str:
    """获取城市的天气"""
    # 通常会调用天气 API
    return f"Weather in {city}: 22°{unit[0].upper()}"
```

#### 结构化输出

工具支持结构化输出，可以返回类型化的数据：

```python
from pydantic import BaseModel, Field
from typing import TypedDict

# 使用 Pydantic 模型
class WeatherData(BaseModel):
    """天气信息结构"""
    temperature: float = Field(description="摄氏温度")
    humidity: float = Field(description="湿度百分比")
    condition: str
    wind_speed: float

@mcp.tool()
def get_weather(city: str) -> WeatherData:
    """获取城市天气 - 返回结构化数据"""
    return WeatherData(
        temperature=22.5,
        humidity=45.0,
        condition="sunny",
        wind_speed=5.2,
    )

# 使用 TypedDict
class LocationInfo(TypedDict):
    latitude: float
    longitude: float
    name: str

@mcp.tool()
def get_location(address: str) -> LocationInfo:
    """获取地理坐标"""
    return LocationInfo(
        latitude=51.5074, 
        longitude=-0.1278, 
        name="London, UK"
    )
```

**支持的返回类型：**
- Pydantic 模型（BaseModel 子类）
- TypedDict
- 数据类和其他带类型提示的类
- `dict[str, T]`（T 是任何 JSON 可序列化类型）
- 原始类型（str, int, float, bool, bytes, None）
- 泛型类型（list, tuple, Union, Optional 等）

#### 带进度报告的工具

```python
from mcp.server.fastmcp import Context, FastMCP

mcp = FastMCP(name="Progress Example")

@mcp.tool()
async def long_running_task(
    task_name: str, 
    ctx: Context, 
    steps: int = 5
) -> str:
    """执行带进度更新的任务"""
    await ctx.info(f"Starting: {task_name}")
    
    for i in range(steps):
        progress = (i + 1) / steps
        await ctx.report_progress(
            progress=progress,
            total=1.0,
            message=f"Step {i + 1}/{steps}",
        )
        await ctx.debug(f"Completed step {i + 1}")
    
    return f"Task '{task_name}' completed"
```

### 4. Prompts (提示词模板)

**Prompts** 是可重用的模板，帮助 LLM 有效地与服务器交互。

```python
from mcp.server.fastmcp import FastMCP
from mcp.server.fastmcp.prompts import base

mcp = FastMCP(name="Prompt Example")

@mcp.prompt(title="Code Review")
def review_code(code: str) -> str:
    return f"Please review this code:\n\n{code}"

@mcp.prompt(title="Debug Assistant")
def debug_error(error: str) -> list[base.Message]:
    return [
        base.UserMessage("I'm seeing this error:"),
        base.UserMessage(error),
        base.AssistantMessage("I'll help debug that. What have you tried so far?"),
    ]
```

**用途：**
- 提供一致的交互模式
- 封装最佳实践
- 简化常见任务
- 支持参数化模板

### 5. Context (上下文)

**Context** 对象自动注入到请求它的工具和资源函数中，提供对 MCP 功能的访问。

#### 获取上下文

```python
from mcp.server.fastmcp import Context, FastMCP

mcp = FastMCP(name="Context Example")

@mcp.tool()
async def my_tool(x: int, ctx: Context) -> str:
    """使用上下文功能的工具"""
    # 上下文参数可以有任何名称，只要有类型注解即可
    return await process_with_context(x, ctx)
```

#### Context 的属性和方法

**属性：**
- `ctx.request_id` - 当前请求的唯一 ID
- `ctx.client_id` - 客户端 ID（如果可用）
- `ctx.fastmcp` - 访问 FastMCP 服务器实例
- `ctx.session` - 访问底层会话进行高级通信
- `ctx.request_context` - 访问请求特定数据和生命周期资源

**方法：**
- `await ctx.debug(message)` - 发送调试日志消息
- `await ctx.info(message)` - 发送信息日志消息
- `await ctx.warning(message)` - 发送警告日志消息
- `await ctx.error(message)` - 发送错误日志消息
- `await ctx.log(level, message, logger_name=None)` - 发送自定义级别日志
- `await ctx.report_progress(progress, total=None, message=None)` - 报告操作进度
- `await ctx.read_resource(uri)` - 通过 URI 读取资源
- `await ctx.elicit(message, schema)` - 请求用户提供额外信息并验证

#### FastMCP 属性

通过 `ctx.fastmcp` 访问服务器配置：

```python
@mcp.tool()
def server_info(ctx: Context) -> dict:
    """获取当前服务器信息"""
    return {
        "name": ctx.fastmcp.name,
        "instructions": ctx.fastmcp.instructions,
        "debug_mode": ctx.fastmcp.settings.debug,
        "log_level": ctx.fastmcp.settings.log_level,
    }
```

#### Session 属性和方法

通过 `ctx.session` 进行高级控制：

- `ctx.session.client_params` - 客户端初始化参数
- `await ctx.session.create_message(messages, max_tokens)` - 请求 LLM 采样/完成
- `await ctx.session.send_resource_updated(uri)` - 通知客户端特定资源已更改
- `await ctx.session.send_resource_list_changed()` - 通知客户端资源列表已更改
- `await ctx.session.send_tool_list_changed()` - 通知客户端工具列表已更改

#### Request Context 属性

通过 `ctx.request_context` 访问请求信息：

- `ctx.request_context.lifespan_context` - 访问启动时初始化的资源
- `ctx.request_context.meta` - 来自客户端的请求元数据
- `ctx.request_context.request` - 原始 MCP 请求对象
- `ctx.request_context.request_id` - 此请求的唯一标识符

### 6. Images (图像)

FastMCP 提供 `Image` 类自动处理图像数据：

```python
from PIL import Image as PILImage
from mcp.server.fastmcp import FastMCP, Image

mcp = FastMCP("Image Example")

@mcp.tool()
def create_thumbnail(image_path: str) -> Image:
    """从图像创建缩略图"""
    img = PILImage.open(image_path)
    img.thumbnail((100, 100))
    return Image(data=img.tobytes(), format="png")
```

### 7. Completions (自动完成)

MCP 支持为提示词参数和资源模板参数提供自动完成建议。

**服务器端示例：**

```python
@mcp.resource("github://{owner}/{repo}")
def get_repo(owner: str, repo: str) -> str:
    """获取 GitHub 仓库信息"""
    return f"Repository: {owner}/{repo}"

# MCP 可以为 owner 和 repo 参数提供自动完成
```

**客户端使用：**

```python
# 不带上下文的完成
result = await session.complete(
    ref=ResourceTemplateReference(type="ref/resource", uri=template.uriTemplate),
    argument={"name": "owner", "value": "model"},
)

# 带上下文的完成 - 基于 owner 建议 repo
result = await session.complete(
    ref=ResourceTemplateReference(type="ref/resource", uri=template.uriTemplate),
    argument={"name": "repo", "value": ""},
    context_arguments={"owner": "modelcontextprotocol"},
)
```

### 8. Elicitation (用户交互请求)

请求用户提供额外信息。支持两种模式：

#### 表单模式（Form Mode）

用于收集结构化的非敏感数据：

```python
from pydantic import BaseModel, Field

class BookingPreferences(BaseModel):
    """收集用户偏好的模式"""
    checkAlternative: bool = Field(description="是否要检查其他日期？")
    alternativeDate: str = Field(
        default="2024-12-26",
        description="备选日期 (YYYY-MM-DD)",
    )

@mcp.tool()
async def book_table(date: str, time: str, party_size: int, ctx: Context) -> str:
    """预订餐桌并检查日期可用性"""
    if date == "2024-12-25":
        # 日期不可用 - 询问用户备选日期
        result = await ctx.elicit(
            message=f"No tables available for {party_size} on {date}. Would you like to try another date?",
            schema=BookingPreferences,
        )
        
        if result.action == "accept" and result.data:
            if result.data.checkAlternative:
                return f"[SUCCESS] Booked for {result.data.alternativeDate}"
            return "[CANCELLED] No booking made"
        return "[CANCELLED] Booking cancelled"
    
    return f"[SUCCESS] Booked for {date} at {time}"
```

#### URL 模式（URL Mode）

用于需要外部交互的操作（如 OAuth、支付确认）：

```python
import uuid
from mcp.shared.exceptions import UrlElicitationRequiredError
from mcp.types import ElicitRequestURLParams

@mcp.tool()
async def secure_payment(amount: float, ctx: Context) -> str:
    """处理需要 URL 确认的安全支付"""
    elicitation_id = str(uuid.uuid4())
    
    result = await ctx.elicit_url(
        message=f"Please confirm payment of ${amount:.2f}",
        url=f"https://payments.example.com/confirm?amount={amount}&id={elicitation_id}",
        elicitation_id=elicitation_id,
    )
    
    if result.action == "accept":
        return f"Payment of ${amount:.2f} initiated"
    return "Payment cancelled"
```

### 9. Sampling (采样)

工具可以通过采样与 LLM 交互（生成文本）：

```python
from mcp.types import SamplingMessage, TextContent

@mcp.tool()
async def generate_poem(topic: str, ctx: Context) -> str:
    """使用 LLM 采样生成诗歌"""
    prompt = f"Write a short poem about {topic}"
    
    result = await ctx.session.create_message(
        messages=[
            SamplingMessage(
                role="user",
                content=TextContent(type="text", text=prompt),
            )
        ],
        max_tokens=100,
    )
    
    if result.content.type == "text":
        return result.content.text
    return str(result.content)
```

### 10. Logging and Notifications (日志和通知)

工具可以通过上下文发送日志和通知：

```python
@mcp.tool()
async def process_data(data: str, ctx: Context) -> str:
    """处理数据并记录日志"""
    # 不同的日志级别
    await ctx.debug(f"Debug: Processing '{data}'")
    await ctx.info("Info: Starting processing")
    await ctx.warning("Warning: This is experimental")
    await ctx.error("Error: (This is just a demo)")
    
    # 通知资源变化
    await ctx.session.send_resource_list_changed()
    
    return f"Processed: {data}"
```

### 11. Authentication (认证)

服务器可以使用 OAuth 2.1 认证保护资源：

```python
from pydantic import AnyHttpUrl
from mcp.server.auth.provider import AccessToken, TokenVerifier
from mcp.server.auth.settings import AuthSettings

class SimpleTokenVerifier(TokenVerifier):
    """简单的令牌验证器示例"""
    async def verify_token(self, token: str) -> AccessToken | None:
        # 这里实现实际的令牌验证
        pass

# 创建带认证的 FastMCP 实例
mcp = FastMCP(
    "Weather Service",
    json_response=True,
    token_verifier=SimpleTokenVerifier(),
    auth=AuthSettings(
        issuer_url=AnyHttpUrl("https://auth.example.com"),
        resource_server_url=AnyHttpUrl("http://localhost:3001"),
        required_scopes=["user"],
    ),
)

@mcp.tool()
async def get_weather(city: str = "London") -> dict[str, str]:
    """获取城市天气数据（需要认证）"""
    return {
        "city": city,
        "temperature": "22",
        "condition": "Partly cloudy",
    }
```

**架构：**
- **Authorization Server (AS)**: 处理 OAuth 流程、用户认证和令牌发放
- **Resource Server (RS)**: 你的 MCP 服务器，验证令牌并提供受保护的资源
- **Client**: 通过 RFC 9728 发现 AS，获取令牌并与 MCP 服务器交互

## MCP 三大基础原语

MCP 协议定义了服务器可以实现的三个核心原语：

| 原语 | 控制方式 | 描述 | 使用示例 |
|------|---------|------|---------|
| **Prompts** | 用户控制 | 由用户选择调用的交互式模板 | 斜杠命令、菜单选项 |
| **Resources** | 应用控制 | 由客户端应用程序管理的上下文数据 | 文件内容、API 响应 |
| **Tools** | 模型控制 | 暴露给 LLM 以执行操作的函数 | API 调用、数据更新 |

## 服务器能力

MCP 服务器在初始化期间声明其能力：

| 能力 | 功能标志 | 描述 |
|------|---------|------|
| `prompts` | `listChanged` | 提示词模板管理 |
| `resources` | `subscribe`<br/>`listChanged` | 资源暴露和更新 |
| `tools` | `listChanged` | 工具发现和执行 |
| `logging` | - | 服务器日志配置 |
| `completions` | - | 参数自动完成建议 |

## 运行服务器

### 开发模式

使用 MCP Inspector 快速测试和调试：

```bash
uv run mcp dev server.py

# 添加依赖
uv run mcp dev server.py --with pandas --with numpy
```

### Claude Desktop 集成

安装到 Claude Desktop：

```bash
uv run mcp install server.py

# 自定义名称
uv run mcp install server.py --name "My Analytics Server"

# 环境变量
uv run mcp install server.py -v API_KEY=abc123
```

### 直接执行

```python
from mcp.server.fastmcp import FastMCP

mcp = FastMCP("My App")

@mcp.tool()
def hello(name: str = "World") -> str:
    """向某人问好"""
    return f"Hello, {name}!"

if __name__ == "__main__":
    mcp.run()
```

运行：

```bash
python server.py
# 或
uv run mcp run server.py
```

### Streamable HTTP 传输

推荐用于生产部署：

```python
from mcp.server.fastmcp import FastMCP

# 无状态服务器（推荐）
mcp = FastMCP("StatelessServer", stateless_http=True, json_response=True)

@mcp.tool()
def greet(name: str = "World") -> str:
    """问候某人"""
    return f"Hello, {name}!"

if __name__ == "__main__":
    mcp.run(transport="streamable-http")
```

## 总结

Model Context Protocol (MCP) Python SDK 提供了一个强大且灵活的框架，用于构建能够与 LLM 无缝集成的服务器。通过理解和使用这些核心概念，你可以：

1. **暴露数据** - 通过 Resources 让 LLM 访问结构化数据
2. **提供功能** - 通过 Tools 让 LLM 执行操作
3. **定义模式** - 通过 Prompts 创建可重用的交互模板
4. **管理上下文** - 使用 Context 访问服务器功能和状态
5. **处理认证** - 使用 OAuth 保护敏感资源
6. **记录日志** - 提供详细的操作日志和通知
7. **交互式请求** - 通过 Elicitation 收集用户输入
8. **与 LLM 交互** - 通过 Sampling 生成内容

MCP 的设计使得构建强大的 LLM 集成变得简单，同时保持代码的清晰和可维护性。无论你是构建简单的数据提供者还是复杂的交互式工具，MCP 都提供了你所需的所有功能。

### 更多资源

- **API 文档**: https://modelcontextprotocol.github.io/python-sdk/api/
- **MCP 协议文档**: https://modelcontextprotocol.io
- **MCP 规范**: https://modelcontextprotocol.io/specification/latest
- **官方服务器示例**: https://github.com/modelcontextprotocol/servers
- **GitHub 仓库**: https://github.com/modelcontextprotocol/python-sdk
