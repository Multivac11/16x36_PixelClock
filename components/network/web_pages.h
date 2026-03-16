// #ifndef WEB_PAGES_H
// #define WEB_PAGES_H

// #include <stdio.h>

// namespace WebPages
// {

//     // 配置页面HTML
//     const char CONFIG_PAGE[] PROGMEM = R"rawliteral(
// <!DOCTYPE html>
// <html>
// <head>
//     <title>ESP32 WiFi配置</title>
//     <meta charset="UTF-8">
//     <meta name="viewport" content="width=device-width, initial-scale=1.0">
//     <style>
//         body {
//             font-family: Arial, sans-serif;
//             max-width: 400px;
//             margin: 0 auto;
//             padding: 20px;
//             background: #f5f5f5;
//         }
//         .container {
//             background: white;
//             padding: 30px;
//             border-radius: 10px;
//             box-shadow: 0 2px 10px rgba(0,0,0,0.1);
//         }
//         h1 {
//             text-align: center;
//             color: #333;
//             margin-bottom: 30px;
//         }
//         .form-group {
//             margin-bottom: 20px;
//         }
//         label {
//             display: block;
//             margin-bottom: 5px;
//             font-weight: bold;
//             color: #555;
//         }
//         select, input[type="password"] {
//             width: 100%;
//             padding: 12px;
//             border: 2px solid #ddd;
//             border-radius: 5px;
//             font-size: 16px;
//             box-sizing: border-box;
//         }
//         select:focus, input[type="password"]:focus {
//             border-color: #4CAF50;
//             outline: none;
//         }
//         .btn {
//             width: 100%;
//             padding: 12px;
//             background: #4CAF50;
//             color: white;
//             border: none;
//             border-radius: 5px;
//             font-size: 16px;
//             cursor: pointer;
//             margin: 5px 0;
//         }
//         .btn:hover {
//             background: #45a049;
//         }
//         .btn-secondary {
//             background: #2196F3;
//         }
//         .btn-secondary:hover {
//             background: #0b7dda;
//         }
//         .loading {
//             display: none;
//             text-align: center;
//             color: #666;
//         }
//         .status {
//             padding: 10px;
//             border-radius: 5px;
//             margin: 10px 0;
//             text-align: center;
//         }
//         .status.success {
//             background: #d4edda;
//             color: #155724;
//             border: 1px solid #c3e6cb;
//         }
//         .status.error {
//             background: #f8d7da;
//             color: #721c24;
//             border: 1px solid #f5c6cb;
//         }
//     </style>
// </head>
// <body>
//     <div class="container">
//         <h1>🛜 PixelClock WiFi配置</h1>

//         <div class="form-group">
//             <button class="btn btn-secondary" onclick="scanNetworks()">📡 扫描WiFi网络</button>
//         </div>

//         <div id="status"></div>

//         <form id="wifiForm" onsubmit="return submitForm()">
//             <div class="form-group">
//                 <label for="ssid">选择WiFi网络:</label>
//                 <select id="ssid" name="ssid" required>
//                     <option value="">-- 请先扫描网络 --</option>
//                 </select>
//             </div>

//             <div class="form-group">
//                 <label for="password">WiFi密码:</label>
//                 <input type="password" id="password" name="password" placeholder="输入WiFi密码" required>
//             </div>

//             <button type="submit" class="btn">🔗 连接WiFi</button>
//         </form>

//         <div id="loading" class="loading">
//             <p>正在连接，请稍候...</p>
//         </div>
//     </div>

//     <script>
//         async function scanNetworks() {
//             const statusDiv = document.getElementById('status');
//             const ssidSelect = document.getElementById('ssid');

//             statusDiv.innerHTML = '<div class="status">扫描中...</div>';
//             ssidSelect.innerHTML = '<option value="">扫描中...</option>';

//             try {
//                 const response = await fetch('/scan');
//                 const networks = await response.json();

//                 // 添加类型检查，确保networks是数组
//                 if (Array.isArray(networks)) {
//                     ssidSelect.innerHTML = '<option value="">-- 选择WiFi网络 --</option>';
//                     networks.forEach(network => {
//                         const option = document.createElement('option');
//                         option.value = network.ssid;
//                         option.textContent = `${network.ssid} (信号强度: ${network.rssi}dBm)`;
//                         ssidSelect.appendChild(option);
//                     });

//                     statusDiv.innerHTML = `<div class="status success">找到 ${networks.length} 个网络</div>`;
//                 } else {
//                     // 如果不是数组，显示适当的错误信息
//                     statusDiv.innerHTML = `<div class="status error">扫描失败: 返回格式错误</div>`;
//                     ssidSelect.innerHTML = '<option value="">扫描失败</option>';
//                 }
//             } catch (error) {
//                 statusDiv.innerHTML = `<div class="status error">扫描失败: ${error}</div>`;
//                 ssidSelect.innerHTML = '<option value="">扫描失败</option>';
//             }
//         }

//         async function submitForm() {
//             const form = document.getElementById('wifiForm');
//             const loading = document.getElementById('loading');
//             const statusDiv = document.getElementById('status');

//             const formData = new FormData(form);
//             const params = new URLSearchParams(formData);

//             loading.style.display = 'block';
//             statusDiv.innerHTML = '';

//             try {
//                 // 使用较长的超时时间，确保连接过程完成
//                 const controller = new AbortController();
//                 const timeoutId = setTimeout(() => controller.abort(), 15000); // 15秒超时

//                 const response = await fetch(`/config?${params}`, {
//                     signal: controller.signal
//                 });
//                 clearTimeout(timeoutId);

//                 if (response.ok) {
//                     const result = await response.json();
//                     if (result.status === 'success') {
//                         // 连接成功，跳转到成功页面
//                         window.location.href = '/success';
//                     } else {
//                         window.location.href = '/failure';
//                     }
//                 } else {
//                     throw new Error('服务器错误');
//                 }
//             } catch (error) {
//                 loading.style.display = 'none';
//                 // 如果是由于AP关闭导致的错误，尝试跳转到成功页面
//                 if (error.name === 'AbortError') {
//                     statusDiv.innerHTML = '<div class="status">连接超时，正在检查连接状态...</div>';
//                     setTimeout(() => {
//                         window.location.href = '/success';
//                     }, 1000);
//                 } else {
//                     statusDiv.innerHTML = `<div class="status error">连接错误: ${error.message}</div>`;
//                 }
//             }

//             return false;
//         }

//         // 页面加载时自动扫描
//         window.onload = scanNetworks;
//     </script>
// </body>
// </html>
// )rawliteral";

//     // 成功页面
//     const char SUCCESS_PAGE[] PROGMEM = R"rawliteral(
// <!DOCTYPE html>
// <html>
// <head>
//     <title>连接成功</title>
//     <meta charset="UTF-8">
//     <meta name="viewport" content="width=device-width, initial-scale=1.0">
//     <style>
//         body {
//             font-family: Arial, sans-serif;
//             max-width: 400px;
//             margin: 0 auto;
//             padding: 20px;
//             background: #f5f5f5;
//             text-align: center;
//         }
//         .container {
//             background: white;
//             padding: 40px 30px;
//             border-radius: 10px;
//             box-shadow: 0 2px 10px rgba(0,0,0,0.1);
//         }
//         .success-icon {
//             font-size: 48px;
//             color: #4CAF50;
//             margin-bottom: 20px;
//         }
//         h1 {
//             color: #4CAF50;
//             margin-bottom: 20px;
//         }
//         .info {
//             background: #e8f5e8;
//             padding: 15px;
//             border-radius: 5px;
//             margin: 20px 0;
//             text-align: left;
//         }
//         .ip-address {
//             font-family: monospace;
//             font-weight: bold;
//             color: #333;
//         }
//         .countdown {
//             font-size: 18px;
//             color: #666;
//             margin: 20px 0;
//         }
//     </style>
// </head>
// <body>
//     <div class="container">
//         <div class="success-icon">✅</div>
//         <h1>连接成功！</h1>

//         <div class="info">
//             <p><strong>ESP32已成功连接到WiFi网络</strong></p>
//             <p>IP地址: <span class="ip-address">%s</span></p>
//         </div>

//         <div class="countdown">
//             <p>AP将在 <span id="countdown">10</span> 秒后关闭...</p>
//         </div>

//         <p>您现在可以关闭此页面</p>
//     </div>

//     <script>
//         // 倒计时
//         let countdown = 10;
//         const countdownElement = document.getElementById('countdown');

//         const timer = setInterval(() => {
//             countdown--;
//             countdownElement.textContent = countdown;

//             if (countdown <= 0) {
//                 clearInterval(timer);
//                 window.close();
//             }
//         }, 1000);
//     </script>
// </body>
// </html>
// )rawliteral";

//     // 失败页面
//     const char FAILURE_PAGE[] PROGMEM = R"rawliteral(
// <!DOCTYPE html>
// <html>
// <head>
//     <title>连接失败</title>
//     <meta charset="UTF-8">
//     <meta name="viewport" content="width=device-width, initial-scale=1.0">
//     <style>
//         body {
//             font-family: Arial, sans-serif;
//             max-width: 400px;
//             margin: 0 auto;
//             padding: 20px;
//             background: #f5f5f5;
//             text-align: center;
//         }
//         .container {
//             background: white;
//             padding: 40px 30px;
//             border-radius: 10px;
//             box-shadow: 0 2px 10px rgba(0,0,0,0.1);
//         }
//         .error-icon {
//             font-size: 48px;
//             color: #f44336;
//             margin-bottom: 20px;
//         }
//         h1 {
//             color: #f44336;
//             margin-bottom: 20px;
//         }
//         .error-info {
//             background: #ffebee;
//             padding: 15px;
//             border-radius: 5px;
//             margin: 20px 0;
//             text-align: left;
//         }
//         .btn {
//             display: inline-block;
//             padding: 12px 24px;
//             margin: 10px;
//             background: #2196F3;
//             color: white;
//             text-decoration: none;
//             border-radius: 5px;
//             border: none;
//             cursor: pointer;
//             font-size: 16px;
//         }
//         .btn:hover {
//             background: #0b7dda;
//         }
//         .btn-secondary {
//             background: #666;
//         }
//         .btn-secondary:hover {
//             background: #555;
//         }
//     </style>
// </head>
// <body>
//     <div class="container">
//         <div class="error-icon">❌</div>
//         <h1>连接失败</h1>

//         <div class="error-info">
//             <p><strong>无法连接到指定的WiFi网络</strong></p>
//             <p>可能原因：</p>
//             <ul>
//                 <li>密码错误</li>
//                 <li>信号强度太弱</li>
//                 <li>网络不可用</li>
//             </ul>
//         </div>

//         <div>
//             <a href="/" class="btn">🔄 重新配置</a>
//         </div>
//     </div>
// </body>
// </html>
// )rawliteral";

// } // namespace WebPages

// #endif