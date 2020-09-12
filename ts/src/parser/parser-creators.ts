import { success, failure } from "./parse-result-creators";
import { Parser } from "./types";

export const mapParserResult = <T>(
  map: (result: string) => T,
  parser: Parser<string>
) => (input: string) => {
  const result = parser(input);
  if (result.kind === "failure") {
    return result;
  }
  return success(map(result.data), result.rest);
};

export const createTextParser = (match: string): Parser<string> => (
  input: string
) => {
  if (input.startsWith(match)) {
    return success(match, input.slice(match.length));
  }
  return failure(match, input);
};

export const createRegexParser = (regex: RegExp): Parser<string> => {
  const anchoredRegex = new RegExp(`^${regex.source}`);
  return (input: string) => {
    const match = anchoredRegex.exec(input);
    if (match === null) {
      return failure(`Regex ${anchoredRegex.source}`, input);
    }
    const matchedText = match[0];
    return success(matchedText, input.slice(matchedText.length));
  };
};
