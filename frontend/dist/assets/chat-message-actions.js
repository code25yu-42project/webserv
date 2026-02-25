(() => {
  if (window.location.pathname !== '/chatroom') return;

  const PROCESSED_ATTR = 'data-ws-message-enhanced';

  const getFiberProps = (element) => {
    const fiberKey = Object.keys(element).find((key) => key.startsWith('__reactFiber$'));
    if (!fiberKey) return null;

    let fiber = element[fiberKey];
    while (fiber) {
      const props = fiber.memoizedProps;
      if (props && props.message && typeof props.message === 'object' && 'content_id' in props.message) {
        return props;
      }
      fiber = fiber.return;
    }
    return null;
  };

  const createActionButton = (label, variant, onClick) => {
    const button = document.createElement('button');
    button.type = 'button';
    button.className = `chat-inline-action ${variant}`;
    button.textContent = label;
    button.addEventListener('click', (event) => {
      event.preventDefault();
      event.stopPropagation();
      onClick();
    });
    return button;
  };

  const buildActions = (props, message) => {
    const actions = document.createElement('div');
    actions.className = 'chat-inline-actions';

    if (!message.is_comment && typeof props.onDownload === 'function') {
      actions.appendChild(createActionButton('Download', 'is-download', () => {
        props.onDownload(message.content_id);
      }));
    }

    if (props.onFurther && typeof props.onDelete === 'function') {
      actions.appendChild(createActionButton('Delete', 'is-delete', () => {
        if (window.confirm('Are you sure you want to delete?')) {
          props.onDelete(message);
        }
      }));
    }

    return actions.childElementCount > 0 ? actions : null;
  };

  const enhanceMessage = (container) => {
    if (!container || container.getAttribute(PROCESSED_ATTR) === '1') return;

    const props = getFiberProps(container);
    if (!props || !props.message || !props.message.is_exist) return;

    const message = props.message;
    const messageBox = container.querySelector('.message-box');
    if (!messageBox) return;

    if (!message.is_comment) {
      const label = messageBox.querySelector('span');
      if (label && label.textContent && label.textContent.includes('File uploaded.')) {
        label.classList.add('chat-file-message-label');
      }
    }

    const actions = buildActions(props, message);
    if (!actions) {
      container.setAttribute(PROCESSED_ATTR, '1');
      return;
    }

    messageBox.classList.add('chat-message-box--with-actions');
    messageBox.appendChild(actions);
    container.setAttribute(PROCESSED_ATTR, '1');
  };

  const runEnhancer = () => {
    const root = document.querySelector('.chat-display');
    if (!root) return;
    root.querySelectorAll('.message-container').forEach(enhanceMessage);
  };

  const clearSelectedFiles = () => {
    document.querySelectorAll('.input-change input[type="file"]').forEach((input) => {
      if (!(input instanceof HTMLInputElement)) return;
      input.value = '';
    });
  };

  const installUploadReset = () => {
    if (window.__wsUploadResetInstalled) return;
    window.__wsUploadResetInstalled = true;

    const isUploadUrl = (url) => typeof url === 'string' && url.includes('/api/chatroom/files');

    const originalOpen = XMLHttpRequest.prototype.open;
    XMLHttpRequest.prototype.open = function (method, url, ...rest) {
      this.__wsUploadUrl = url;
      return originalOpen.call(this, method, url, ...rest);
    };

    const originalSend = XMLHttpRequest.prototype.send;
    XMLHttpRequest.prototype.send = function (...args) {
      this.addEventListener('loadend', () => {
        if (isUploadUrl(this.__wsUploadUrl) && this.status >= 200 && this.status < 300) {
          clearSelectedFiles();
        }
      }, { once: true });
      return originalSend.apply(this, args);
    };

    if (typeof window.fetch === 'function') {
      const originalFetch = window.fetch;
      window.fetch = async (...args) => {
        const response = await originalFetch(...args);
        const requestUrl = typeof args[0] === 'string'
          ? args[0]
          : (args[0] && typeof args[0].url === 'string' ? args[0].url : '');

        if (isUploadUrl(requestUrl) && response && response.ok) {
          clearSelectedFiles();
        }

        return response;
      };
    }
  };

  let rafId = null;
  const scheduleEnhancer = () => {
    if (rafId !== null) return;
    rafId = requestAnimationFrame(() => {
      rafId = null;
      runEnhancer();
    });
  };

  const initObserver = () => {
    const root = document.querySelector('.chat-display');
    if (!root) {
      setTimeout(initObserver, 200);
      return;
    }

    const blockMessagePopup = (event) => {
      const target = event.target;
      if (!(target instanceof Element)) return;

      const actionButton = target.closest('.chat-inline-action');
      if (actionButton) return;

      const messageContainer = target.closest('.message-container');
      if (!messageContainer) return;

      event.preventDefault();
      event.stopPropagation();
    };

    root.addEventListener('click', blockMessagePopup, true);

    const observer = new MutationObserver(() => scheduleEnhancer());
    observer.observe(root, { childList: true, subtree: true });
    scheduleEnhancer();
  };

  try {
    installUploadReset();
    initObserver();
  } catch (error) {
    console.error('chat-message-actions init failed:', error);
  }
})();
