/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// CLASS HEADER
#include <dali-toolkit/internal/text/shaper.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/shaping.h>

namespace Dali
{

namespace Toolkit
{

namespace Text
{

CharacterIndex min( CharacterIndex index0,
                    CharacterIndex index1 )
{
  return ( index0 < index1 ) ? index0 : index1;
}

void ShapeText( const Vector<Character>& text,
                const Vector<LineBreakInfo>& lineBreakInfo,
                const Vector<ScriptRun>& scripts,
                const Vector<FontRun>& fonts,
                Vector<GlyphInfo>& glyphs,
                Vector<CharacterIndex>& glyphToCharacterMap,
                Vector<Length>& charactersPerGlyph,
                Vector<GlyphIndex>& newParagraphGlyphs )
{
  const Length numberOfCharacters = text.Count();

  if( 0u == numberOfCharacters )
  {
    // Nothing to do if there are no characters.
    return;
  }

#ifdef DEBUG_ENABLED
  const Length numberOfFontRuns = fonts.Count();
  const Length numberOfScriptRuns = scripts.Count();
#endif

  DALI_ASSERT_DEBUG( ( 0u != numberOfFontRuns ) &&
                     ( numberOfCharacters == fonts[numberOfFontRuns - 1u].characterRun.characterIndex + fonts[numberOfFontRuns - 1u].characterRun.numberOfCharacters ) &&
                     "Toolkit::Text::ShapeText. All characters must have a font set." );

  DALI_ASSERT_DEBUG( ( 0u != numberOfScriptRuns ) &&
                     ( numberOfCharacters == scripts[numberOfScriptRuns - 1u].characterRun.characterIndex + scripts[numberOfScriptRuns - 1u].characterRun.numberOfCharacters ) &&
                     "Toolkit::Text::ShapeText. All characters must have a script set." );

  // The text needs to be split in chunks of consecutive characters.
  // Each chunk must contain characters with the same font id and script set.
  // A chunk of consecutive characters must not contain a LINE_MUST_BREAK, if there is one a new chunk has to be created.

  TextAbstraction::Shaping shaping = TextAbstraction::Shaping::Get();

  // To shape the text a font and an script is needed.
  Vector<FontRun>::ConstIterator fontRunIt = fonts.Begin();
  Vector<ScriptRun>::ConstIterator scriptRunIt = scripts.Begin();

  // Index to the the next one to be shaped. Is pointing the character after the last one it was shaped.
  CharacterIndex previousIndex = 0u;

  // The current font id and script used to shape the text.
  FontId currentFontId = 0u;
  Script currentScript = TextAbstraction::UNKNOWN;

  // Reserve some space to allocate the glyphs and the glyph to character map.
  // There is no way to know the number of glyphs before shaping the text.
  // To avoid reallocations it's reserved space for a slightly biger number of glyphs than the number of characters.

  Length numberOfGlyphsReserved = static_cast<Length>( numberOfCharacters * 1.3f );
  glyphs.Resize( numberOfGlyphsReserved );
  glyphToCharacterMap.Resize( numberOfGlyphsReserved );

  // The actual number of glyphs.
  Length totalNumberOfGlyphs = 0u;

  const Character* const textBuffer = text.Begin();
  const LineBreakInfo* const lineBreakInfoBuffer = lineBreakInfo.Begin();
  GlyphInfo* glyphsBuffer = glyphs.Begin();
  CharacterIndex* glyphToCharacterMapBuffer = glyphToCharacterMap.Begin();

  // Traverse the characters and shape the text.
  for( previousIndex = 0; previousIndex < numberOfCharacters; )
  {
    // Get the font id and the script.
    const FontRun& fontRun = *fontRunIt;
    const ScriptRun& scriptRun = *scriptRunIt;

    currentFontId = fontRun.fontId;
    currentScript = scriptRun.script;

    // Get the min index to the last character of both runs.
    CharacterIndex currentIndex = min( fontRun.characterRun.characterIndex + fontRun.characterRun.numberOfCharacters,
                                       scriptRun.characterRun.characterIndex + scriptRun.characterRun.numberOfCharacters );

    // Check if there is a line must break.
    bool mustBreak = false;

    // Check if the current index is a new paragraph character.
    // A new paragraph character is going to be shaped in order to not to mess the conversion tables.
    // However, the metrics need to be changed in order to not to draw a square.
    bool isNewParagraph = false;

    for( CharacterIndex index = previousIndex; index < currentIndex; ++index )
    {
      mustBreak = TextAbstraction::LINE_MUST_BREAK == *( lineBreakInfoBuffer + index );
      if( mustBreak )
      {
        isNewParagraph = TextAbstraction::IsNewParagraph( *( textBuffer + index ) );
        currentIndex = index + 1u;
        break;
      }
    }

    // Shape the text for the current chunk.
    const Length numberOfGlyphs = shaping.Shape( textBuffer + previousIndex,
                                                 ( currentIndex - previousIndex ), // The number of characters to shape.
                                                 currentFontId,
                                                 currentScript );

    const Length glyphIndex = totalNumberOfGlyphs;
    totalNumberOfGlyphs += numberOfGlyphs;

    if( totalNumberOfGlyphs > numberOfGlyphsReserved )
    {
      // Resize the vectors to get enough space.
      numberOfGlyphsReserved = static_cast<Length>( totalNumberOfGlyphs * 1.3f );
      glyphs.Resize( numberOfGlyphsReserved );
      glyphToCharacterMap.Resize( numberOfGlyphsReserved );

      // Iterators are not valid anymore, set them again.
      glyphsBuffer = glyphs.Begin();
      glyphToCharacterMapBuffer = glyphToCharacterMap.Begin();
    }

    // Retrieve the glyphs and the glyph to character conversion map.
    shaping.GetGlyphs( glyphsBuffer + glyphIndex,
                       glyphToCharacterMapBuffer + glyphIndex );

    if( isNewParagraph )
    {
      // Add the index of the new paragraph glyph to a vector.
      // Their metrics will be updated in a following step.
      newParagraphGlyphs.PushBack( totalNumberOfGlyphs - 1u );
    }

    // Update indices.
    if( 0u != glyphIndex )
    {
      for( Length index = glyphIndex; index < glyphIndex + numberOfGlyphs; ++index )
      {
        CharacterIndex& characterIndex = *( glyphToCharacterMapBuffer + index );
        characterIndex += previousIndex;
      }
    }

    // Update the iterators to get the next font or script run.
    if( currentIndex == fontRun.characterRun.characterIndex + fontRun.characterRun.numberOfCharacters )
    {
      ++fontRunIt;
    }
    if( currentIndex == scriptRun.characterRun.characterIndex + scriptRun.characterRun.numberOfCharacters )
    {
      ++scriptRunIt;
    }

    // Update the previous index.
    previousIndex = currentIndex;
  }

  // Add the number of characters per glyph.
  charactersPerGlyph.Reserve( totalNumberOfGlyphs );
  previousIndex = 0u;
  for( Length index = 1u; index < totalNumberOfGlyphs; ++index )
  {
    const CharacterIndex characterIndex = *( glyphToCharacterMapBuffer + index );

    charactersPerGlyph.PushBack( characterIndex - previousIndex );

    previousIndex = characterIndex;
  }

  charactersPerGlyph.PushBack( numberOfCharacters - previousIndex );

  // Resize the vectors to set the right number of items.
  glyphs.Resize( totalNumberOfGlyphs );
  glyphToCharacterMap.Resize( totalNumberOfGlyphs );
}

} // namespace Text

} // namespace Toolkit

} // namespace Dali
