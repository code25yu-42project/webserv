(function () {
  const MAX_LOGS = 150;
  const monitorState = {
    logs: [],
    filter: 'all',
    collapsed: false,
    paused: false
  };

  const chatNoticeState = {
    initialized: false,
    users: new Set(),
    selfJoinShown: false
  };

  function safeStringify(value) {
    if (value == null) return '';
    if (typeof value === 'string') return value;
    if (typeof value === 'object') {
      try {
        return JSON.stringify(value);
      } catch (error) {
        return '[unserializable]';
      }
    }
    return String(value);
  }

  function trimText(text, limit) {
    if (!text) return '';
    return text.length > limit ? text.slice(0, limit) + ' ...' : text;
  }

  function getLogCategory(url, method) {
    if (!url) return 'http';
    const urlStr = String(url).toLowerCase();
    if (urlStr.includes('/files') || urlStr.includes('download') || urlStr.includes('upload')) return 'file';
    if (urlStr.includes('/comments')) return 'message';
    return 'http';
  }

  function formatBodyPreview(body, isRequest) {
    if (!body) return '';
    const limit = isRequest ? 300 : 500;
    const text = safeStringify(body);
    if (!text || text === '{}' || text === '[]') return '';
    return trimText(text, limit);
  }

  function normalizeUrl(url) {
    if (!url) return '';
    try {
      const parsed = new URL(String(url), window.location.origin);
      return parsed.pathname + parsed.search;
    } catch (error) {
      const raw = String(url);
      const schemeIndex = raw.indexOf('://');
      if (schemeIndex !== -1) {
        const pathIndex = raw.indexOf('/', schemeIndex + 3);
        return pathIndex !== -1 ? raw.slice(pathIndex) : '/';
      }
      return raw;
    }
  }

  function ensureMonitorUI() {
    if (document.querySelector('.ws-monitor')) return;

    const monitor = document.createElement('aside');
    monitor.className = 'ws-monitor';
    monitor.innerHTML = [
      '<button class="ws-monitor__toggle" type="button" aria-label="Toggle monitor">⮜</button>',
      '<div class="ws-monitor__header">',
      '  <div class="ws-monitor__title-wrap">',
      '    <span class="ws-monitor__title">webserv LOG</span>',
      '    <span class="ws-monitor__status">LIVE</span>',
      '  </div>',
      '  <div class="ws-monitor__actions">',
      '    <button class="ws-monitor__pause" type="button">PAUSE</button>',
      '    <button class="ws-monitor__clear" type="button">CLEAR</button>',
      '  </div>',
      '</div>',
      '<div class="ws-monitor__filters">',
      '  <button class="ws-monitor__filter is-active" data-filter="all" type="button">ALL</button>',
      '  <button class="ws-monitor__filter" data-filter="http" type="button">HTTP</button>',
      '  <button class="ws-monitor__filter" data-filter="file" type="button">FILE</button>',
      '  <button class="ws-monitor__filter" data-filter="message" type="button">MSG</button>',
      '</div>',
      '<ul class="ws-monitor__list"></ul>'
    ].join('');

    const clearButton = monitor.querySelector('.ws-monitor__clear');
    const pauseButton = monitor.querySelector('.ws-monitor__pause');
    const toggleButton = monitor.querySelector('.ws-monitor__toggle');
    const filters = monitor.querySelectorAll('.ws-monitor__filter');

    clearButton.addEventListener('click', function () {
      monitorState.logs = [];
      renderMonitorList();
    });

    pauseButton.addEventListener('click', function () {
      monitorState.paused = !monitorState.paused;
      monitor.classList.toggle('is-paused', monitorState.paused);
      pauseButton.textContent = monitorState.paused ? 'RESUME' : 'PAUSE';
      const status = monitor.querySelector('.ws-monitor__status');
      if (status) status.textContent = monitorState.paused ? 'PAUSED' : 'LIVE';
      if (!monitorState.paused) renderMonitorList();
    });

    toggleButton.addEventListener('click', function () {
      monitorState.collapsed = !monitorState.collapsed;
      document.body.classList.toggle('ws-monitor-collapsed', monitorState.collapsed);
      monitor.classList.toggle('is-collapsed', monitorState.collapsed);
      toggleButton.textContent = monitorState.collapsed ? '⮞' : '⮜';
    });

    filters.forEach(function (btn) {
      btn.addEventListener('click', function () {
        monitorState.filter = btn.getAttribute('data-filter') || 'all';
        renderMonitorList();
      });
    });

    document.body.appendChild(monitor);
    renderMonitorList();
  }

  function renderMonitorList() {
    const monitor = document.querySelector('.ws-monitor');
    if (!monitor) return;

    const list = monitor.querySelector('.ws-monitor__list');
    const filters = monitor.querySelectorAll('.ws-monitor__filter');

    if (!list) return;

    filters.forEach(function (btn) {
      const filter = btn.getAttribute('data-filter');
      btn.classList.toggle('is-active', filter === monitorState.filter);
    });

    list.innerHTML = '';

    const filteredLogs = monitorState.logs.filter(function (entry) {
      if (monitorState.filter === 'all') return true;
      return entry.category === monitorState.filter;
    });

    filteredLogs.forEach(function (entry) {
      const item = document.createElement('li');
      item.className = 'ws-monitor__item ws-monitor__item--' + entry.category;

      const statusClass = entry.statusCode 
        ? (entry.statusCode < 400 ? 'success' : entry.statusCode < 500 ? 'warn' : 'error')
        : 'info';

      const parts = [
        '<div class="ws-monitor__meta">' +
        '<span class="ws-monitor__badge ws-monitor__badge--' + statusClass + '">' + entry.type + '</span> ' +
        '<span class="ws-monitor__method">' + entry.method + '</span> ' +
        '<span class="ws-monitor__url">' + trimText(entry.url, 60) + '</span> ' +
        '<span class="ws-monitor__time">' + entry.timeText + '</span>' +
        '</div>'
      ];

      if (entry.statusCode) {
        parts.push('<div class="ws-monitor__status">' + entry.statusCode + 
          (entry.durationMs ? ' · ' + entry.durationMs + 'ms' : '') + '</div>');
      }

      if (entry.detail) {
        parts.push('<div class="ws-monitor__detail">' + entry.detail + '</div>');
      }

      if (entry.bodyPreview) {
        parts.push('<div class="ws-monitor__body">' + entry.bodyPreview + '</div>');
      }

      item.innerHTML = parts.join('');
      list.appendChild(item);
    });
  }

  function appendLog(type, method, url, detail, options) {
    options = options || {};
    const now = new Date();
    const normalizedUrl = normalizeUrl(url);
    const category = options.category || getLogCategory(normalizedUrl, method);
    const bodyPreview = options.bodyPreview || '';

    monitorState.logs.unshift({
      type: type,
      method: method.toUpperCase(),
      url: normalizedUrl,
      detail: detail,
      category: category,
      timeText: now.toLocaleTimeString("en-US"),
      statusCode: options.statusCode || null,
      durationMs: options.durationMs || null,
      bodyPreview: bodyPreview
    });

    while (monitorState.logs.length > MAX_LOGS) {
      monitorState.logs.pop();
    }

    if (!monitorState.paused) {
      renderMonitorList();
    }
  }

  function patchFetch() {
    if (!window.fetch || window.__wsFetchPatched) return;
    window.__wsFetchPatched = true;

    const originalFetch = window.fetch.bind(window);

    window.fetch = async function (input, init) {
      const method = (init && init.method) || 'GET';
      const url = typeof input === 'string' ? input : (input && input.url) || '';
      const normalizedUrl = normalizeUrl(url);
      const started = performance.now();
      const reqBody = init && init.body ? init.body : null;

      let reqBodyPreview = '';
      if (reqBody) {
        if (typeof reqBody === 'string') {
          reqBodyPreview = formatBodyPreview(reqBody, true);
        } else if (reqBody instanceof FormData) {
          reqBodyPreview = '[FormData]';
        } else {
          reqBodyPreview = formatBodyPreview(reqBody, true);
        }
      }

      appendLog('REQ', method, normalizedUrl, '', {
        bodyPreview: reqBodyPreview
      });

      try {
        const response = await originalFetch(input, init);
        const cloned = response.clone();
        let responseText = '';
        try {
          responseText = await cloned.text();
        } catch (error) {
          responseText = '';
        }
        const duration = Math.round(performance.now() - started);

        let detail = '';
        if (normalizedUrl.includes('/files') && method === 'POST') {
          detail = '📤 File Upload';
        } else if (normalizedUrl.includes('/download')) {
          detail = '📥 File Download';
        } else if (normalizedUrl.includes('/files') && method === 'DELETE') {
          detail = '🗑️ File Delete';
        } else if (normalizedUrl.includes('/comments') && method === 'POST') {
          detail = '💬 Message Post';
        } else if (normalizedUrl.includes('/comments') && method === 'DELETE') {
          detail = '🗑️ Message Delete';
        }

        const resBodyPreview = formatBodyPreview(responseText, false);

        appendLog('RES', method, normalizedUrl, detail, {
          statusCode: response.status,
          durationMs: duration,
          category: getLogCategory(normalizedUrl, method),
          bodyPreview: resBodyPreview
        });

        syncChatroomUserNotices(url, responseText);
        return response;
      } catch (error) {
        const duration = Math.round(performance.now() - started);
        appendLog('ERR', method, normalizedUrl, error && error.message ? error.message : 'Network error', {
          durationMs: duration,
          category: getLogCategory(normalizedUrl, method)
        });
        throw error;
      }
    };
  }

  function patchXHR() {
    if (!window.XMLHttpRequest || window.__wsXHRPatched) return;
    window.__wsXHRPatched = true;

    const originalOpen = XMLHttpRequest.prototype.open;
    const originalSend = XMLHttpRequest.prototype.send;

    XMLHttpRequest.prototype.open = function (method, url) {
      this.__wsMethod = method;
      this.__wsUrl = url;
      return originalOpen.apply(this, arguments);
    };

    XMLHttpRequest.prototype.send = function (body) {
      this.__wsStarted = performance.now();
      const method = (this.__wsMethod || 'GET').toUpperCase();
      const normalizedUrl = normalizeUrl(this.__wsUrl || '');

      let reqBodyPreview = '';
      if (body) {
        if (typeof body === 'string') {
          reqBodyPreview = formatBodyPreview(body, true);
        } else if (body instanceof FormData) {
          reqBodyPreview = '[FormData]';
        } else {
          reqBodyPreview = formatBodyPreview(body, true);
        }
      }

      appendLog('REQ', method, normalizedUrl, '', {
        bodyPreview: reqBodyPreview
      });

      this.addEventListener('load', () => {
        const methodName = (this.__wsMethod || 'GET').toUpperCase();
        const duration = this.__wsStarted ? Math.round(performance.now() - this.__wsStarted) : null;
        const url = this.__wsUrl || '';
        const normalizedResUrl = normalizeUrl(url);

        let detail = '';
        if (normalizedResUrl.includes('/files') && methodName === 'POST') {
          detail = '📤 File Upload';
        } else if (normalizedResUrl.includes('/download')) {
          detail = '📥 File Download';
        } else if (normalizedResUrl.includes('/files') && methodName === 'DELETE') {
          detail = '🗑️ File Delete';
        } else if (normalizedResUrl.includes('/comments') && methodName === 'POST') {
          detail = '💬 Message Post';
        } else if (normalizedResUrl.includes('/comments') && methodName === 'DELETE') {
          detail = '🗑️ Message Delete';
        }

        const resBodyPreview = formatBodyPreview(this.responseText || '', false);

        appendLog('RES', methodName, normalizedResUrl, detail, {
          statusCode: this.status,
          durationMs: duration,
          category: getLogCategory(normalizedResUrl, methodName),
          bodyPreview: resBodyPreview
        });

        syncChatroomUserNotices(url, this.responseText || '');
      });

      this.addEventListener('error', () => {
        const methodName = (this.__wsMethod || 'GET').toUpperCase();
        const url = this.__wsUrl || '';
        const normalizedErrUrl = normalizeUrl(url);
        const duration = this.__wsStarted ? Math.round(performance.now() - this.__wsStarted) : null;
        appendLog('ERR', methodName, normalizedErrUrl, 'XHR failed', {
          durationMs: duration,
          category: getLogCategory(normalizedErrUrl, methodName)
        });
      });

      return originalSend.apply(this, arguments);
    };
  }

  function routePath() {
    return window.location.pathname || '';
  }

  function getStoredUserName() {
    const raw = localStorage.getItem('wsUserName');
    if (!raw) return '';
    return String(raw);
  }

  function getStoredUserId() {
    const raw = localStorage.getItem('userInfo');
    if (!raw) return null;
    try {
      const parsed = JSON.parse(raw);
      const value = Number(parsed);
      return Number.isFinite(value) ? value : null;
    } catch (error) {
      const value = Number(raw);
      return Number.isFinite(value) ? value : null;
    }
  }

  function hasValidUserId() {
    const id = getStoredUserId();
    return id != null && id > 0;
  }

  function allowNextChatroomEntry() {
    try {
      sessionStorage.setItem('wsAllowChatroomEntry', '1');
      localStorage.setItem('wsShowSelfJoinNotice', '1');
    } catch (error) {}
  }

  function consumeChatroomEntryAllowance() {
    try {
      const value = sessionStorage.getItem('wsAllowChatroomEntry');
      if (value === '1') {
        sessionStorage.removeItem('wsAllowChatroomEntry');
        return true;
      }
    } catch (error) {}
    return false;
  }

  function isChatroomTrafficUrl(url) {
    if (!url) return false;
    return String(url).indexOf('/chatroom') !== -1;
  }

  function syncGlobalHeaderVisibility() {
    const nav = document.querySelector('.navbar');
    const footer = document.querySelector('.btn-navbar');
    if (!nav) return;

    if (routePath() === '/main') {
      nav.style.display = 'none';
      if (footer) footer.style.display = 'none';
    } else {
      nav.style.display = '';
      if (footer) footer.style.display = '';
    }
  }

  function renderMainCenter() {
    if (routePath() !== '/main') return;

    const target = document.querySelector('.main-container');
    if (!target || target.dataset.wsMainPatched === '1') return;

    target.dataset.wsMainPatched = '1';
    target.innerHTML = [
      '<section class="ws-main-center">',
      '  <div class="ws-main-center__box ws-main-hero">',
      '    <div class="ws-main-hero__left">',
      '      <p class="ws-main-hero__eyebrow">42 WEB SERVER</p>',
      '      <h1 class="ws-main-center__title">webserv Control Lounge</h1>',
      '      <p class="ws-main-center__desc">',
      '        Observe requests and responses in real time, ',
      '        This is a test space to verify server behavior immediately via chat.',
      '      </p>',
      '      <div class="ws-main-entry">',
      '        <input class="ws-main-entry__input" type="text" placeholder="Enter nickname" />',
      '        <button class="ws-main-center__btn ws-main-entry__btn" type="button">Enter</button>',
      '      </div>',
      '      <p class="ws-main-entry__error" style="display:none;"></p>',
      '    </div>',
      '    <div class="ws-main-hero__right">',
      '      <div class="ws-main-kpi">',
      '        <span class="ws-main-kpi__label">Tech Stack</span>',
      '        <strong class="ws-main-kpi__value">C++98 · HTTP/1.1 · Kqueue</strong>',
      '      </div>',
      '      <div class="ws-main-kpi">',
      '        <span class="ws-main-kpi__label">Core Features</span>',
      '        <strong class="ws-main-kpi__value">CGI · Upload · WebSocket</strong>',
      '      </div>',
      '      <div class="ws-main-kpi">',
      '        <span class="ws-main-kpi__label">Architecture</span>',
      '        <strong class="ws-main-kpi__value">Event-driven · Non-blocking</strong>',
      '      </div>',
      '    </div>',
      '  </div>',
      '</section>'
    ].join('');

    const entryInput = target.querySelector('.ws-main-entry__input');
    const entryButton = target.querySelector('.ws-main-entry__btn');
    const errorBox = target.querySelector('.ws-main-entry__error');

    const mainNotice = localStorage.getItem('wsMainNotice');
    if (mainNotice) {
      errorBox.textContent = mainNotice;
      errorBox.style.display = 'block';
      localStorage.removeItem('wsMainNotice');
    }

    entryInput.value = getStoredUserName();

    async function handleMainEntry() {
      const userName = (entryInput.value || '').trim();

      entryButton.disabled = true;
      errorBox.style.display = 'none';

      try {
        appendLog('USER', 'REGISTER', '/api/users/register', userName || '[random-name]');

        const requestBody = userName ? { user_name: userName } : {};

        const response = await fetch('/api/users/register', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify(requestBody)
        });

        if (!response.ok) {
          throw new Error('Failed to register user_name.');
        }

        const payload = await response.json();
        const userId = payload && payload.body ? payload.body.user_id : null;

        if (userId == null || userId === '') {
          throw new Error('Failed to receive registered user_id.');
        }

        localStorage.setItem('userInfo', JSON.stringify(Number(userId)));
        const resolvedName = payload && payload.body && payload.body.user_name ? String(payload.body.user_name) : userName;
        if (resolvedName) {
          localStorage.setItem('wsUserName', resolvedName);
        } else {
          localStorage.removeItem('wsUserName');
        }

        appendLog('USER', 'REGISTER_OK', '/api/users/register', 'user_id=' + String(userId));

        const redirectUrl = payload && payload.redirect ? payload.redirect.redirect_url : null;
        if (redirectUrl && typeof redirectUrl === 'string') {
          allowNextChatroomEntry();
          window.location.href = redirectUrl;
          return;
        }

        allowNextChatroomEntry();
        window.location.href = '/chatroom';
      } catch (error) {
        errorBox.textContent = error && error.message ? error.message : 'Registration failed.';
        errorBox.style.display = 'block';
      } finally {
        entryButton.disabled = false;
      }
    }

    entryButton.addEventListener('click', handleMainEntry);
    entryInput.addEventListener('keydown', function (event) {
      if (event.key === 'Enter') {
        event.preventDefault();
        handleMainEntry();
      }
    });
  }

  function renderChatroomLayout() {
    if (routePath() !== '/chatroom') return;

    const hasAllowance = consumeChatroomEntryAllowance();

    if (!hasAllowance && !hasValidUserId()) {
      localStorage.setItem('wsMainNotice', 'Please register your username first to enter the chatroom.');
      window.location.replace('/main');
      return;
    }

    if (!hasValidUserId()) {
      localStorage.setItem('wsMainNotice', 'Please register your username first to enter the chatroom.');
      window.location.replace('/main');
      return;
    }

    const chatContainer = document.querySelector('.chat-container');
    const textarea = document.querySelector('textarea');
    if (!chatContainer || !textarea) return;

    const pageRoot = chatContainer.parentElement;
    if (!pageRoot) return;

    if (pageRoot.dataset.wsChatPatched !== '1') {
      pageRoot.dataset.wsChatPatched = '1';
      pageRoot.classList.add('ws-chat-page');

      const header = document.createElement('section');
      header.className = 'ws-chat-header';
      header.innerHTML = [
        '<div class="ws-chat-header__top">',
        '  <h2 class="ws-chat-header__title">Chatroom Mission Control</h2>',
        '  <span class="ws-chat-header__chip">real-time test</span>',
        '</div>',
        '<p class="ws-chat-header__desc">Verify behavior by viewing message/file actions alongside server logs.</p>'
      ].join('');

      pageRoot.insertBefore(header, chatContainer);
    }

    chatContainer.classList.add('ws-chat-scroll');
    const display = chatContainer.querySelector('.chat-display');
    if (display) display.classList.add('ws-chat-display');

    const composer = textarea.closest('div');
    if (composer) {
      composer.classList.add('ws-chat-composer');
    }

    textarea.classList.add('ws-chat-input');

    const fileInput = document.querySelector('.input-change input[type="file"]');
    if (fileInput) fileInput.classList.add('ws-chat-file');

    const sendButton = document.querySelector('.input-change button');
    if (sendButton) {
      sendButton.classList.add('ws-chat-send');
      sendButton.style.position = 'static';
      sendButton.style.right = '';
    }

    const showSelfNotice = localStorage.getItem('wsShowSelfJoinNotice') === '1';
    if (showSelfNotice && !chatNoticeState.selfJoinShown) {
      const currentUserId = getStoredUserId();
      const userName = getStoredUserName() || (currentUserId != null ? ('user#' + String(currentUserId)) : 'user');
      appendChatNotice(userName + ' joined.', 'join');
      chatNoticeState.selfJoinShown = true;
      localStorage.removeItem('wsShowSelfJoinNotice');
    }
  }

  function appendChatNotice(message, type) {
    if (routePath() !== '/chatroom') return;
    const display = document.querySelector('.chat-display');
    if (!display) return;

    const notice = document.createElement('div');
    notice.className = 'ws-chat-notice ' + (type === 'leave' ? 'ws-chat-notice--leave' : 'ws-chat-notice--join');
    const timeText = new Date().toLocaleTimeString("en-US");
    notice.textContent = '[' + timeText + '] ' + message;

    display.appendChild(notice);
    display.scrollTop = display.scrollHeight;

    const notices = display.querySelectorAll('.ws-chat-notice');
    if (notices.length > 20) {
      const first = notices[0];
      if (first) first.remove();
    }
  }

  function syncChatroomUserNotices(url, responseText) {
    if (!url || url.indexOf('/api/chatroom') === -1) return;
    if (!responseText) return;

    let payload;
    try {
      payload = JSON.parse(responseText);
    } catch (error) {
      return;
    }

    const users = payload && Array.isArray(payload.users) ? payload.users : null;
    if (!users) return;

    const nextSet = new Set();
    const names = new Map();

    users.forEach(function (user) {
      if (!user || user.user_id == null) return;
      const id = String(user.user_id);
      nextSet.add(id);
      names.set(id, user.user_name || ('user#' + id));
    });

    if (!chatNoticeState.initialized) {
      chatNoticeState.users = nextSet;
      chatNoticeState.initialized = true;
      return;
    }

    nextSet.forEach(function (id) {
      if (!chatNoticeState.users.has(id)) {
        appendChatNotice(names.get(id) + ' joined.', 'join');
      }
    });

    chatNoticeState.users.forEach(function (id) {
      if (!nextSet.has(id)) {
        appendChatNotice('user#' + id + ' left.', 'leave');
      }
    });

    chatNoticeState.users = nextSet;
  }

  function renderRouteLayouts() {
    syncGlobalHeaderVisibility();
    renderMainCenter();
    renderChatroomLayout();
  }

  function installRouteHooks() {
    const originalPushState = history.pushState;
    const originalReplaceState = history.replaceState;

    history.pushState = function () {
      const result = originalPushState.apply(this, arguments);
      appendLog('USER', 'ROUTE', window.location.pathname, 'route change');
      setTimeout(renderRouteLayouts, 0);
      return result;
    };

    history.replaceState = function () {
      const result = originalReplaceState.apply(this, arguments);
      appendLog('USER', 'ROUTE', window.location.pathname, 'route replace');
      setTimeout(renderRouteLayouts, 0);
      return result;
    };

    window.addEventListener('popstate', function () {
      appendLog('USER', 'ROUTE', window.location.pathname, 'route popstate');
      setTimeout(renderRouteLayouts, 0);
    });

    const observer = new MutationObserver(function () {
      renderRouteLayouts();
    });

    observer.observe(document.body, { childList: true, subtree: true });
  }

  function init() {
    if (document.body) {
      document.body.classList.add('ws-monitor-ready');
    }
    ensureMonitorUI();
    patchFetch();
    patchXHR();
    installRouteHooks();
    renderRouteLayouts();
    appendLog('INIT', 'webserv', 'Monitor initialized', 'system');
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }
})();
