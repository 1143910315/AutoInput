// weiui.d.ts
declare global {
  interface Window {
    webui: {
      call: (eventName: string, ...restArgs: unknown[]) => Promise<string>;
      encode: (str: string) => string;
      decode: (base64: string) => string;
      setLogging: (enabled: boolean) => void;
    };
  }
}

// 确保将该文件导出（通常为空导出）
export {};
